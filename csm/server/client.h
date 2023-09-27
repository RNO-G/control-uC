#ifndef RNO_G_CLIENT_H
#define RNO_G_CLIENT_H

#define RNO_G_CLIENT_BUF_SIZE 512

#include <fcntl.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

typedef struct client client;

client * client_create(int uart, int * run, int * busy,
                       int * num_connections_queued, pthread_cond_t * client_cond,
                       pthread_mutex_t * client_mutex,
                       pthread_mutex_t * connection_queue_mutex,
                       pthread_mutex_t * uart_mutex,
                       void (* sig_handler) (int),
                       int (* connection_queue_dequeue) ());

int client_destroy(client * inst);

void * client_run(void * inst_prtr);

#endif
