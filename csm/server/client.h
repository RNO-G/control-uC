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

// data structure: client
typedef struct client client;

/*
 * function: client_create
 * -----------------------
 * create a new client instance
 *
 * uart: the file descriptor for the uart
 *
 * run: a pointer to the run status variable for the client
 *
 * busy: a pointer to the busy status variable for the client (i.e. if a client is actively handling a connection or not)
 *
 * num_connections_queued: a pointer to the number of connections that are queued in the connection queue
 *
 * client_cond: a pointer to the pthread_cond that is used by the client to sleep until a connection is served by the main thread
 *
 * client_mutex: a pointer to the mutex that is called when operating on the run or busy pointers
 *
 * connection_queue_mutex: a pointer to the mutex that is called when operating on the connection queue
 *
 * uart_mutex: a pointer to the mutex that is called when operating on the uart
 *
 * sig_handler: a pointer to the client signal handler function
 *
 * connection_queue_dequeue: a pointer to a function that gets the connection at the top of the queue
 *
 * returns: a pointer to a new client instance, NULL if an error occurred
 */
client * client_create(int uart, int * run, int * busy,
                       int * num_connections_queued, pthread_cond_t * client_cond,
                       pthread_mutex_t * client_mutex,
                       pthread_mutex_t * connection_queue_mutex,
                       pthread_mutex_t * uart_mutex,
                       void (* sig_handler) (int),
                       int (* connection_queue_dequeue) ());

/*
 * function: client_destroy
 * ------------------------
 * destroy a client instance
 *
 * inst: a pointer to the client instance
 *
 * returns: 0 if the instance was successfully destroyed, a nonzero integer otherwise
 */
int client_destroy(client * inst);

/*
 * function: client_run
 * --------------------
 * initiate a run loop for the client instance (this function is supplied to pthread_create)
 *
 * inst_ptr: a void pointer to the client instance (will be casted to a client pointer)
 *
 * returns: nothing
 */
void * client_run(void * inst_ptr);

#endif
