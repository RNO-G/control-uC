#ifndef RNO_G_SERVER_H
#define RNO_G_SERVER_H

#define RNO_G_CLIENT_LIM 10

#include <arpa/inet.h>
#include <ctype.h>
#include <errno.h>

#include "client.h"

// data structure: server 
typedef struct server server;

/*
 * function: server_create
 * -----------------------
 * create a new server instance
 *
 * addr: a string corresponding to the server's IP address
 *
 * port: a string corresponding to the server's port number
 *
 * uart_path: a string corresponding to the path to the uart
 *
 * run: a pointer to the run status variable for the server
 *
 * client_run: an array containing the run status variables for each of the client threads
 *
 * client_busy: an array containing the busy status variables for each of the client threads
 *
 * client_mutex: an array containing the client mutexes for each of the client threads
 *
 * thread_pool: an array containing the pthread_t data structures for each of the client threads
 *
 * num_connections_queued: a pointer to the number of queued connections
 *
 * connection_queue_mutex: a pointer to the connection queue mutex
 *
 * server_sig_handler: a pointer to the server signal handler function
 *
 * connection_queue_enqueue: a pointer to the function that adds a connection to the queue
 *
 * client_sig_handler: a pointer to the client signal handler function (this is not stored; it is passed to the clients)
 *
 * connection_queue_dequeue: a pointer to the function that gets the connection at the top of the queue (this is not stored; it is passed to the clients)
 *
 * returns: a pointer to a new server instance; NULL if an error occurred
 */
server * server_create(char * addr, char * port, char * uart_path, int * run,
                       int client_run[RNO_G_CLIENT_LIM], int client_busy[RNO_G_CLIENT_LIM],
                       pthread_mutex_t client_mutex[RNO_G_CLIENT_LIM], pthread_t thread_pool[RNO_G_CLIENT_LIM],
                       int * num_connections_queued, pthread_mutex_t * connection_queue_mutex,
                       void (* server_sig_handler) (int), void (* connection_queue_enqueue) (int),
                       void (* client_sig_handler) (int), int (* connection_queue_dequeue) ());

/*
 * function: server_destroy
 * ------------------------
 * destroys a server instance
 *
 * inst: a pointer to the server instance
 *
 * returns: 0 if the instance was successfully destroyed, a nonzero integer otherwise
 */
int server_destroy(server * inst);

/*
 * function: server_run
 * --------------------
 * initiate the run loop for the server instance
 *
 * inst: a pointer to the server instance
 *
 * returns: 0 if the run loop terminated successfully, a nonzero integer otherwise
 */
int server_run(server * inst);

#endif
