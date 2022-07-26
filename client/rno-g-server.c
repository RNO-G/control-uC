#include "rno-g-server.h"

int server_socket;

char cmd_queue[CMD_LIM][BUF_SIZE];
int cmd_queue_running = 1;
size_t cmd_queue_len = 0;
pthread_t cmd_queue_manager;
pthread_mutex_t cmd_queue_mutex;


void signal_handler(int sig) {
    if (sig == SIGINT) {
        cmd_queue_running = 0;

        error_check(close(server_socket));
        error_check(pthread_join(cmd_queue_manager, NULL));
        error_check(pthread_mutex_destroy(&cmd_queue_mutex));
        exit(EXIT_SUCCESS);
    }
}

int cmd_enqueue(char * cmd) {
    if (cmd_queue_len == CMD_LIM) {
        return -1;
    }
    else {
        strcpy(cmd_queue[cmd_queue_len], cmd);
        cmd_queue_len++;
    }

    return 0;
}

int cmd_dequeue() {
    if (cmd_queue_len == 0) {
        return -1;
    }
    else {
        for (int i = 1; i < CMD_LIM; i++) {
            strcpy(cmd_queue[i - 1], cmd_queue[i]);
        }

        cmd_queue_len--;
    }

    return 0;
}

int cmd_flush() {
    memset(cmd_queue, 0, sizeof(char) * CMD_LIM * BUF_SIZE);
    
    return 0;
}

void print_cmd_queue() {
    fputs("COMMAND QUEUE : ", stdout);
    
    for (int i = 0; i < cmd_queue_len; i++) {
        fputs("{", stdout);
        fputs(cmd_queue[i], stdout);
        fputs("} ", stdout);
    }

    fputs("\n", stdout);
}

void * manage_cmd_queue() {
    char * cmd;
    
    while (cmd_queue_running) {
        pthread_mutex_lock(&cmd_queue_mutex);

        print_cmd_queue();

        if (cmd_queue_len > 0) {
            cmd = cmd_queue[0];
            cmd_dequeue();

            if (!strcmp(cmd, "LTE-OFF")) {
                fputs("FLUSHING COMMAND QUEUE\n", stdout);
                cmd_flush();

                cmd_queue_running = 0;
            }
        }

        pthread_mutex_unlock(&cmd_queue_mutex);

        sleep(5);
    }

    pthread_exit(EXIT_SUCCESS);
}

void * manage_client(void * client_socket_ptr) {
    int client_socket = *((int *) client_socket_ptr);

    char cmd[BUF_SIZE];
    char ack[BUF_SIZE] = "Command Recieved\n";
    
    while (1) {
        error_check(read(client_socket, cmd, BUF_SIZE));

        if (cmd_queue_running) {
            pthread_mutex_lock(&cmd_queue_mutex);
            
            if (cmd_enqueue(cmd) == -1) {
                fputs("SERVER COMMAND QUEUE FULL, TRY AGAIN LATER\n", stderr);
            }
            
            pthread_mutex_unlock(&cmd_queue_mutex);
        }
        else {
            fputs("SERVER ASLEEP\n", stdout);
        }

        error_check(write(client_socket, ack, BUF_SIZE));

        if (!strcmp(cmd, "DISCONNECT")) {
            break;
        }
    }

    error_check(close(client_socket));
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv) {
    int client_socket;
        
    pthread_t client;
    pthread_attr_t attr;
    
    struct sockaddr_in server_addr;

    memset(cmd_queue, 0, sizeof(char) * CMD_LIM * BUF_SIZE);    

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    error_check(server_socket = socket(AF_INET, SOCK_STREAM, 0));
    error_check(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)));
    error_check(listen(server_socket, CLI_LIM));
    error_check(pthread_mutex_init(&cmd_queue_mutex, NULL));
    error_check(pthread_create(&cmd_queue_manager, NULL, manage_cmd_queue, NULL));

    signal(SIGINT, signal_handler);

    while (1) {
        error_check(client_socket = accept(server_socket, (struct sockaddr *) NULL, NULL));
        
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, 1);
        pthread_create(&client, &attr, manage_client, &client_socket);
        pthread_attr_destroy(&attr);
    }

    cmd_queue_running = 0;
    
    error_check(close(server_socket));
    error_check(pthread_join(cmd_queue_manager, NULL));
    error_check(pthread_mutex_destroy(&cmd_queue_mutex));

    return EXIT_SUCCESS;
}
