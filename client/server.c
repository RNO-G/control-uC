#include "server.h"
#include "constants.h"

struct thd_status {
    int thd_running;
    int cli_running;
    pthread_cond_t thd_cond;
};

int svr_running;
int num_cli;
int uart;
int cli_queue[QUEUED_CLIENT_LIM];
pthread_t thd_pool[ACTIVE_CLIENT_LIM];
thd_status thd_pool_status[ACTIVE_CLIENT_LIM];
pthread_mutex_t cli_queue_mutex;

void main_sig_handler(int sig) {
    if (sig == SIGINT) {
        svr_running = 0;
    }
}

void cli_sig_handler(int sig) {
    if (sig == SIGUSR1) { 
        pthread_t tid = pthread_self();
        for (int i = 0; i < ACTIVE_CLIENT_LIM; i++) {
            if (pthread_equal(thd_pool[i], tid)) {
                thd_pool_status[i].thd_running = 0;
                thd_pool_status[i].cli_running = 0;
                break;
            }
        }
    }
}

void cli_queue_enqueue(int cli_sock) {
    cli_queue[num_cli] = cli_sock;
    num_cli++;
}

int cli_queue_dequeue() {
    int cli_sock = cli_queue[0];

    for (int i = 0; i < num_cli - 1; i++) {
        cli_queue[i] = cli_queue[i + 1];
    }

    num_cli--;

    return cli_sock;
}

void manage_cli(int cli_sock) {
    int len;
    char cmd[BUF_SIZE] = {'\0'};
    char cpy[BUF_SIZE] = {'\0'};
    char ack[BUF_SIZE] = {'\0'};
    
    while (1) {
        if (read(cli_sock, cmd, BUF_SIZE) < 1) {
            break;
        }
        else {
            len = strlen(cmd);
            if (len == BUF_SIZE - 1) {
                strcpy(ack, "COMMAND TOO LONG");
            }
            else {
                strcpy(cpy, cmd);
                
                cmd[0] = '#';
                cmd[len + 2] = '\0';

                for (int i = 1; i < len + 1; i++) {
                    cmd[i] = cpy[i - 1];
                }

                flock(uart, LOCK_EX);
                errno_check(write(uart, cmd, BUF_SIZE), "write");
                errno_check(read(uart, ack, BUF_SIZE), "read");
                flock(uart, LOCK_UN);
            }
        }
        
        if (write(cli_sock, ack, BUF_SIZE) < 1) {
            break;
        }
    }
}

void * manage_thd(void * status) {
    int cli_sock;

    struct sigaction sig = {.sa_flags = 0, .sa_handler = cli_sig_handler};

    errno_check(sigaction(SIGUSR1, &sig, NULL), "sigaction");

    while (((thd_status *) status)->thd_running) {
        errno_check(pthread_mutex_lock(&cli_queue_mutex), "pthread_mutex_lock");
        errno_check(pthread_cond_wait(&((thd_status *) status)->thd_cond, &cli_queue_mutex), "pthread_cond_wait");
        while (num_cli > 0 && ((thd_status *) status)->thd_running) {
            cli_sock = cli_queue_dequeue();
            errno_check(pthread_mutex_unlock(&cli_queue_mutex), "pthread_mutex_unlock");
            
            ((thd_status *) status)->cli_running = 1;
            manage_cli(cli_sock); 
            ((thd_status *) status)->cli_running = 0;
            
            errno_check(pthread_mutex_lock(&cli_queue_mutex), "pthread_mutex_lock");
        }
        
        errno_check(pthread_mutex_unlock(&cli_queue_mutex), "pthread_mutex_unlock");
    }

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv) {
    int cli_sock, svr_sock;
    struct sockaddr_in svr_addr;
    struct sigaction ign, sig;
    sigset_t set;

    svr_running = 1;
    num_cli = 0;

    if (argc == 1) {
        errno_check(access(argv[0], F_OK), "access");
        uart = open(argv[0], O_RDWR);
    }
    else {
        fprintf(stderr, "INVALID NUMBER OF ARGUMENTS");
    }

    svr_addr.sin_family = AF_INET;
    svr_addr.sin_port = htons(PORT);
    svr_addr.sin_addr.s_addr = INADDR_ANY;

    errno_check(pthread_mutex_init(&cli_queue_mutex, NULL), "pthread_mutex_init");

    errno_check(svr_sock = socket(AF_INET, SOCK_STREAM, 0), "socket");
    errno_check(bind(svr_sock, (struct sockaddr *) &svr_addr, sizeof(svr_addr)), "bind");
    errno_check(listen(svr_sock, ACTIVE_CLIENT_LIM), "listen");

    errno_check(sigemptyset(&set), "sigemptyset");
    errno_check(sigaddset(&set, SIGINT), "sigaddset");
    errno_check(sigaddset(&set, SIGPIPE), "sigaddset");
    errno_check(pthread_sigmask(SIG_BLOCK, &set, NULL), "pthread_sigmask");

    for (int i = 0; i < ACTIVE_CLIENT_LIM; i++) {
        thd_pool_status[i].thd_running = 1;
        thd_pool_status[i].cli_running = 0;
        errno_check(pthread_cond_init(&thd_pool_status[i].thd_cond, NULL), "pthread_cond_init");
        errno_check(pthread_create(&thd_pool[i], NULL, manage_thd, (void *) &thd_pool_status[i]), "pthread_create");
    }

    errno_check(sigemptyset(&set), "sigemptyset");
    errno_check(sigaddset(&set, SIGINT), "sigaddset");
    errno_check(sigaddset(&set, SIGPIPE), "sigaddset");
    errno_check(pthread_sigmask(SIG_UNBLOCK, &set, NULL), "pthread_sigmask");
    
    explicit_bzero(&ign, sizeof(struct sigaction));
    explicit_bzero(&sig, sizeof(struct sigaction));

    ign.sa_flags = 0;
    ign.sa_handler = SIG_IGN;

    sig.sa_flags = 0;
    sig.sa_handler = main_sig_handler;

    errno_check(sigaction(SIGPIPE, &ign, NULL), "sigaction");
    errno_check(sigaction(SIGUSR1, &ign, NULL), "sigaction");
    errno_check(sigaction(SIGINT, &sig, NULL), "sigaction");
    
    while(svr_running) {
        cli_sock = accept(svr_sock, NULL, NULL);
        
        if (cli_sock == -1) {
            if (!svr_running) {
                break;
            } 
            else {
                errno_check(cli_sock, "accept");
            }
        }

        errno_check(pthread_mutex_lock(&cli_queue_mutex), "pthread_mutex_lock");
        
        if (num_cli == QUEUED_CLIENT_LIM) {
            printf("TOO MANY CLIENTS, TRY CONNECTING LATER\n");
            errno_check(close(cli_sock), "close");
        }
        else {
            cli_queue_enqueue(cli_sock);
        }

        errno_check(pthread_mutex_unlock(&cli_queue_mutex), "pthread_mutex_unlock");
        
        for (int i = 0; i < ACTIVE_CLIENT_LIM; i++) {
            if (thd_pool_status[i].cli_running == 0) {
                errno_check(pthread_cond_signal(&thd_pool_status[i].thd_cond), "pthread_cond_signal");
                break;
            }
        }
    }

    for (int i = 0; i < ACTIVE_CLIENT_LIM; i++) {
        errno_check(pthread_kill(thd_pool[i], SIGUSR1), "pthread_kill");
        errno_check(pthread_cond_signal(&thd_pool_status[i].thd_cond), "pthread_cond_signal");
        errno_check(pthread_join(thd_pool[i], NULL), "pthread_join");
        errno_check(pthread_cond_destroy(&thd_pool_status[i].thd_cond), "pthread_cond_destroy");
    }
    
    errno_check(pthread_mutex_destroy(&cli_queue_mutex), "pthread_mutex_destroy");

    for (int i = 0; i < num_cli; i++) {
        errno_check(close(cli_queue[i]), "close");
    }

    errno_check(close(svr_sock), "close");

    return EXIT_SUCCESS;
}

