#include "client.h"

struct client {
    int socket;
    int uart;
    int * run;
    int * busy;
    int * num_connections_queued;
    char buf[RNO_G_CLIENT_BUF_SIZE+1];
    char ack[RNO_G_CLIENT_BUF_SIZE+1];
    pthread_cond_t * client_cond;
    pthread_mutex_t * client_mutex;
    pthread_mutex_t * connection_queue_mutex;
    pthread_mutex_t * uart_mutex;
    void (* sig_handler) (int);
    int (* connection_queue_dequeue) ();
};

client * client_create(int uart, int * run, int * busy,
                       int * num_connections_queued, pthread_cond_t * client_cond,
                       pthread_mutex_t * client_mutex,
                       pthread_mutex_t * connection_queue_mutex,
                       pthread_mutex_t * uart_mutex,
                       void (* sig_handler) (int),
                       int (* connection_queue_dequeue) ()) {

    if (fcntl(uart, F_GETFD) == -1) {
        perror("client_create");
        return NULL;
    }

    if (run == NULL) {
        fprintf(stderr, "client_create: run status pointer must not be null");
        return NULL;
    }

    if (busy == NULL) {
        fprintf(stderr, "client_create: busy status pointer must not be null");
        return NULL;
    }

    if (num_connections_queued == NULL) {
        fprintf(stderr, "client_create: number of queued connections pointer must not be null");
        return NULL;
    }

    if (client_cond == NULL) {
        fprintf(stderr, "client_create: client cond must not be null");
        return NULL;
    }

    if (client_mutex == NULL) {
        fprintf(stderr, "client_create: client mutex must not be null");
        return NULL;
    }

    if (connection_queue_mutex == NULL) {
        fprintf(stderr, "client_create: connection queue mutex must not be null");
        return NULL;
    }

    if (uart_mutex == NULL) {
        fprintf(stderr, "client_create: uart mutex must not be null");
        return NULL;
    }

    if (sig_handler == NULL) {
        fprintf(stderr, "client_create: signal handler function must not be null");
        return NULL;
    }

    if (connection_queue_dequeue == NULL) {
        fprintf(stderr, "client_create: connection queue dequeue function must not be null");
        return NULL;
    }

    client * inst = (client *) malloc(sizeof(client));

    if (inst == NULL) {
        perror("client_create");
        free(inst);
        return NULL;
    }

    memset(inst->buf, 0, RNO_G_CLIENT_BUF_SIZE+1);
    memset(inst->ack, 0, RNO_G_CLIENT_BUF_SIZE+1);

    inst->uart = uart;
    inst->run = run;
    inst->busy = busy;
    inst->num_connections_queued = num_connections_queued;
    inst->client_cond = client_cond;
    inst->client_mutex = client_mutex;
    inst->connection_queue_mutex = connection_queue_mutex;
    inst->uart_mutex = uart_mutex;
    inst->sig_handler = sig_handler;
    inst->connection_queue_dequeue = connection_queue_dequeue;

    return inst;
}

int client_destroy(client * inst) {
    free(inst);
    return EXIT_SUCCESS;
}

void manage_connection(client * inst) {
    size_t len;

    char cpy[RNO_G_CLIENT_BUF_SIZE+1];

    while (1) {
        if (read(inst->socket, inst->buf, RNO_G_CLIENT_BUF_SIZE+1) < 1) {
            if (close(inst->socket) == -1) {
                perror("manage_connection");
                pthread_exit(NULL);
            }

            break;
        }
        else {
            len = strlen(inst->buf);

            if (len == RNO_G_CLIENT_BUF_SIZE - 1) {
                strcpy(inst->ack, "COMMAND TOO LONG");
            }
            else {
                strcpy(cpy, inst->buf);

                inst->buf[0] = '#';
                inst->buf[len + 1] = '\n';
                inst->buf[len + 2] = '\0';

                for (size_t i = 0; i < len; i++) {
                    inst->buf[i + 1] = cpy[i];
                }

                if (pthread_mutex_lock(inst->uart_mutex)) {
                    perror("manage_connection");

                    if (close(inst->socket) == -1) {
                        perror("manage_connection");
                        pthread_exit(NULL);
                    }

                    pthread_exit(NULL);
                }

                if (write(inst->uart, inst->buf, RNO_G_CLIENT_BUF_SIZE+1) == -1) {
                    perror("manage_connection");
                    printf("here");

                    if (pthread_mutex_unlock(inst->uart_mutex)) {
                        perror("manage_connection");

                        if (close(inst->socket) == -1) {
                            perror("manage_connection");
                            pthread_exit(NULL);
                        }

                        pthread_exit(NULL);
                    }

                    if (close(inst->socket) == -1) {
                        perror("manage_connection");
                        pthread_exit(NULL);
                    }

                    pthread_exit(NULL);
                }

                if (read(inst->uart, inst->ack, RNO_G_CLIENT_BUF_SIZE+1) == -1) {
                    perror("manage_connection");

                    if (pthread_mutex_unlock(inst->uart_mutex)) {
                        perror("manage_connection");

                        if (close(inst->socket) == -1) {
                            perror("manage_connection");
                            pthread_exit(NULL);
                        }

                        pthread_exit(NULL);
                    }

                    if (close(inst->socket) == -1) {
                        perror("manage_connection");
                        pthread_exit(NULL);
                    }

                    pthread_exit(NULL);
                }

                if (pthread_mutex_unlock(inst->uart_mutex)) {
                    perror("manage_connection");

                    if (close(inst->socket) == -1) {
                        perror("manage_connection");
                        pthread_exit(NULL);
                    }

                    pthread_exit(NULL);
                }
            }
        }

        if (write(inst->socket, inst->ack, RNO_G_CLIENT_BUF_SIZE+1) < 1) {
            if (close(inst->socket) == -1) {
                perror("manage_connection");
                pthread_exit(NULL);
            }

            break;
        }

        memset(inst->buf, 0, RNO_G_CLIENT_BUF_SIZE+1);
        memset(inst->ack, 0, RNO_G_CLIENT_BUF_SIZE+1);
    }

    if (close(inst->socket) == -1) {
        perror("manage_connection");
        pthread_exit(NULL);
    }
}

void * client_run(void * inst_ptr) {
    if (inst_ptr == NULL) {
        fprintf(stderr, "client_run: client must be provided sufficient data");
        pthread_exit(NULL);
    }

    client * inst = (client *) inst_ptr;

    struct sigaction sig = {.sa_flags = 0, .sa_handler = inst->sig_handler};

    if (sigaction(SIGUSR1, &sig, NULL)) {
        perror("client_run");
        pthread_exit(NULL);
    }

    while (1) {
        if (pthread_mutex_lock(inst->connection_queue_mutex)) {
            perror("client_run");
            pthread_exit(NULL);
        }

        if (pthread_cond_wait(inst->client_cond, inst->connection_queue_mutex)) {
            perror("client_run");
            pthread_exit(NULL);
        }

        while (1) {
            if (pthread_mutex_lock(inst->client_mutex)) {
                perror("client_run");

                if (pthread_mutex_unlock(inst->connection_queue_mutex)) {
                    perror("client_run");
                    pthread_exit(NULL);
                }

                pthread_exit(NULL);
            }

            if (*(inst->num_connections_queued) && *(inst->run) == 1) {
                inst->socket = inst->connection_queue_dequeue();

                if (pthread_mutex_unlock(inst->connection_queue_mutex)) {
                    perror("client_run");

                    if (pthread_mutex_unlock(inst->client_mutex)) {
                        perror("client_run");
                        pthread_exit(NULL);
                    }

                    pthread_exit(NULL);
                }

                *(inst->busy) = 1;

                if (pthread_mutex_unlock(inst->client_mutex)) {
                    perror("client_run");
                    pthread_exit(NULL);
                }

                printf("thread managing socket\n");
                manage_connection(inst);

                if (pthread_mutex_lock(inst->client_mutex)) {
                    perror("client_run");
                    pthread_exit(NULL);
                }

                *(inst->busy) = 0;

                if (pthread_mutex_unlock(inst->client_mutex)) {
                    perror("client_run");
                    pthread_exit(NULL);
                }

                if (pthread_mutex_lock(inst->connection_queue_mutex)) {
                    perror("client_run");
                    pthread_exit(NULL);
                }
            }
            else {
                if (pthread_mutex_unlock(inst->connection_queue_mutex)) {
                    perror("client_run");

                    if (pthread_mutex_unlock(inst->client_mutex)) {
                        perror("client_run");
                        pthread_exit(NULL);
                    }

                    pthread_exit(NULL);
                }

                if (pthread_mutex_unlock(inst->client_mutex)) {
                    perror("client_run");
                    pthread_exit(NULL);
                }

                break;
            }
        }

        if (pthread_mutex_unlock(inst->connection_queue_mutex)) {
            perror("client_run");

            if (pthread_mutex_unlock(inst->client_mutex)) {
                perror("client_run");
                pthread_exit(NULL);
            }

            pthread_exit(NULL);
        }

        if (pthread_mutex_lock(inst->client_mutex)) {
            perror("client_run");
            pthread_exit(NULL);
        }

        if (*(inst->run)) {
            if (pthread_mutex_unlock(inst->client_mutex)) {
                perror("client_run");
                pthread_exit(NULL);
            }

            continue;
        }
        else {
            if (pthread_mutex_unlock(inst->client_mutex)) {
                perror("client_run");
                pthread_exit(NULL);
            }

            break;
        }
    }

    pthread_exit(NULL);
}
