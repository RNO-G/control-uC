#include "server.h"
#include "constants.h"

int num_clients, num_cmd;
int client_queue[QUEUED_CLIENT_LIM];
char cmd_queue[CMD_LIM][BUF_SIZE];
int thread_running[ACTIVE_CLIENT_LIM];
pthread_t thread_pool[ACTIVE_CLIENT_LIM];
pthread_mutex_t client_queue_mutex, cmd_queue_mutex;

int cmd_queue_running = 1;
int server_running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        server_running = 0;
    }
    else if (sig == SIGUSR1) {
        pthread_t tid = pthread_self();
        int i;
        for (i = 0; i < ACTIVE_CLIENT_LIM; i++) {
            if (pthread_equal(thread_pool[i], tid)) {
                break;
            }
        }
        thread_running[i] = 0;
    }
}

void client_queue_enqueue(int client_socket) {
    client_queue[num_clients] = client_socket;
    num_clients++;
}

int client_queue_dequeue() {
    int client_socket = client_queue[0];

    for (int i = 0; i < num_clients - 1; i++) {
        client_queue[i] = client_queue[i + 1];
    }

    num_clients--;

    return client_socket;
}

void cmd_queue_enqueue(char * cmd) {
    strcpy(cmd_queue[num_cmd], cmd);
    num_cmd++;
}

void cmd_queue_dequeue(char * cmd) {
    strcpy(cmd, cmd_queue[0]);
    
    for (int i = 0; i < num_cmd - 1; i++) {
        strcpy(cmd_queue[i], cmd_queue[i + 1]);
    }

    num_cmd--;
}

void * cmd_queue_manager() {
    char cmd[BUF_SIZE];

    memset(cmd, 0, sizeof(char) * BUF_SIZE);

    while (cmd_queue_running) {
        errno_check(pthread_mutex_lock(&cmd_queue_mutex), "pthread_mutex_lock");

        if (num_cmd > 0) {
            cmd_queue_dequeue(cmd);
            printf("%s\n", cmd);
        }

        errno_check(pthread_mutex_unlock(&cmd_queue_mutex), "pthread_mutex_unlock");
    }

    pthread_exit(EXIT_SUCCESS);
}

void manage_client(int client_socket) {
    char cmd[BUF_SIZE];
    char ack[BUF_SIZE];
    
    memset(cmd, 0, sizeof(char) * BUF_SIZE);
    memset(ack, 0, sizeof(char) * BUF_SIZE);

    while (1) {
        if (read(client_socket, cmd, BUF_SIZE) < 1) {
            break;
        }
        else {
            errno_check(pthread_mutex_lock(&cmd_queue_mutex), "pthread_mutex_lock");

            if (num_cmd == CMD_LIM) {
                strcpy(ack, "COMMAND QUEUE FULL, TRY AGAIN LATER");
            }
            else {
                strcpy(ack, "COMMAND RECIEVED");
                cmd_queue_enqueue(cmd);
                num_cmd++;
            }

            errno_check(pthread_mutex_unlock(&cmd_queue_mutex), "pthread_mutex_unlock");
        }
        
        if (write(client_socket, ack, BUF_SIZE) < 1) {
            break;
        }
    }
}

void * manage_thread(void * running) {
    int client_socket;
    struct sigaction sig;

    memset(&sig, 0, sizeof(struct sigaction));

    sig.sa_flags = 0;
    sig.sa_handler = signal_handler;

    errno_check(sigaction(SIGUSR1, &sig, NULL), "sigaction");

    while (*((int *) running)) {
        errno_check(pthread_mutex_lock(&client_queue_mutex), "pthread_mutex_lock");
        if (num_clients > 0) {
            client_socket = client_queue_dequeue();
            errno_check(pthread_mutex_unlock(&client_queue_mutex), "pthread_mutex_unlock");
            manage_client(client_socket);
        }
        else {
            errno_check(pthread_mutex_unlock(&client_queue_mutex), "pthread_mutex_unlock");
        }
    }

    pthread_exit(EXIT_SUCCESS);
}

int main() {
    int client_socket, server_socket;
    struct sockaddr_in server_addr;
    struct sigaction ign, sig;
    sigset_t set;
    pthread_t cmd_queue_manager_thread;

    num_clients = 0;
    num_cmd = 0;

    memset(client_queue, 0, sizeof(int) * QUEUED_CLIENT_LIM);
    memset(cmd_queue, 0, sizeof(char) * CMD_LIM * BUF_SIZE);
    memset(thread_running, 0, sizeof(int) * ACTIVE_CLIENT_LIM);
    memset(thread_pool, 0, sizeof(pthread_t) * ACTIVE_CLIENT_LIM);

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    errno_check(pthread_mutex_init(&client_queue_mutex, NULL), "pthread_mutex_init");
    errno_check(pthread_mutex_init(&cmd_queue_mutex, NULL), "pthread_mutex_init");

    errno_check(server_socket = socket(AF_INET, SOCK_STREAM, 0), "socket");
    errno_check(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)), "bind");
    errno_check(listen(server_socket, ACTIVE_CLIENT_LIM), "listen");

    errno_check(sigemptyset(&set), "sigemptyset");
    errno_check(sigaddset(&set, SIGINT), "sigaddset");
    errno_check(sigaddset(&set, SIGPIPE), "sigaddset");
    errno_check(pthread_sigmask(SIG_BLOCK, &set, NULL), "pthread_sigmask");

    errno_check(pthread_create(&cmd_queue_manager_thread, NULL, cmd_queue_manager, NULL), "pthread_create");

    for (int i = 0; i < ACTIVE_CLIENT_LIM; i++) {
        thread_running[i] = 1;
        errno_check(pthread_create(&thread_pool[i], NULL, manage_thread, (void *) &thread_running[i]), "pthread_create");
    }

    errno_check(sigemptyset(&set), "sigemptyset");
    errno_check(sigaddset(&set, SIGINT), "sigaddset");
    errno_check(sigaddset(&set, SIGPIPE), "sigaddset");
    errno_check(pthread_sigmask(SIG_UNBLOCK, &set, NULL), "pthread_sigmask");
    
    memset(&ign, 0, sizeof(struct sigaction));
    memset(&sig, 0, sizeof(struct sigaction));

    ign.sa_flags = 0;
    ign.sa_handler = SIG_IGN;

    sig.sa_flags = 0;
    sig.sa_handler = signal_handler;

    errno_check(sigaction(SIGPIPE, &ign, NULL), "sigaction");
    errno_check(sigaction(SIGUSR1, &ign, NULL), "sigaction");
    errno_check(sigaction(SIGINT, &sig, NULL), "sigaction");

    while(server_running) {
        client_socket = accept(server_socket, NULL, NULL);
        errno_check(pthread_mutex_lock(&client_queue_mutex), "pthread_mutex_lock");
        
        if (num_clients == QUEUED_CLIENT_LIM) {
            printf("TOO MANY CLIENTS, TRY CONNECTING LATER\n");
            errno_check(close(client_socket), "close");
        }
        else {
            client_queue_enqueue(client_socket);
            num_clients++;
        }

        errno_check(pthread_mutex_unlock(&client_queue_mutex), "pthread_mutex_unlock");
    }

    cmd_queue_running = 0;
    pthread_join(cmd_queue_manager_thread, NULL);

    for (int i = 0; i < ACTIVE_CLIENT_LIM; i++) {
        thread_running[i] = 0;
        pthread_kill(thread_pool[i], SIGUSR1);
        pthread_join(thread_pool[i], NULL);
    }

    for (int i = 0; i < num_clients; i++) {
        errno_check(close(client_queue[i]), "close");
    }

    close(server_socket);

    return EXIT_SUCCESS;
}

