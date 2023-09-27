#ifndef RNO_G_SERVER_H
#define RNO_G_SERVER_H

#define RNO_G_CLIENT_LIM 10

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>

#include "client.h"

typedef struct server server;

server * server_create(char * addr, char * port, char * uart_path, int * run,
                       int client_run[RNO_G_CLIENT_LIM], int client_busy[RNO_G_CLIENT_LIM],
                       pthread_mutex_t client_mutex[RNO_G_CLIENT_LIM], pthread_t thread_pool[RNO_G_CLIENT_LIM],
                       int * num_connections_queued, pthread_mutex_t * connection_queue_mutex,
                       void (* server_sig_handler) (int), void (* connection_queue_enqueue) (int),
                       void (* client_sig_handler) (int), int (* connection_queue_dequeue) ());

int server_destroy(server * inst);

int server_run(server * inst);

#endif
