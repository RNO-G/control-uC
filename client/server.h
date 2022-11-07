#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <sys/file.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "error.h"

/*
 * struct : cli_status
 * -------------------
 * a structure to contain the run status and condition for a client
 * thread
 *
 * thread_running : the run status
 *
 * thread_cond : the condition
 */
typedef struct thd_status thd_status;

/*
 * function : main_signal_handler
 * ------------------------------
 * handle system signals recieved by the main thread
 * 
 * sig : the signal
 * 
 * returns : nothing
 */
void main_sig_handler(int sig);

/*
 * function : client_thread_signal_handler
 * ---------------------------------------
 * handle system signals recieved by a client thread
 * 
 * sig : the signal
 * 
 * returns : nothing
 */
void cli_sig_handler(int sig);

/*
 * function : client_queue_enqueue
 * -------------------------------
 * add a pending client connection to the client queue
 *
 * client_socket : the client socket
 *
 * returns : nothing
 */
void cli_queue_enqueue(int client_socket);

/*
 * function : client_queue_dequeue
 * -------------------------------
 * remove the head client connection from the client queue
 *
 * returns : the client socket at the head of the queue
 */
int cli_queue_dequeue();

/*
 * function : manage_client
 * ------------------------
 * handler function for managing a client connection
 *
 * client_socket : the client connection
 *
 * returns : nothing
 */
void manage_cli(int client_socket);

/*
 * function : manage_thread
 * ------------------------
 * thread handler function for managing the thread pool
 *
 * running : the pointer to the run status of the thread
 *
 * returns : thread exit status
 */
void * manage_thd(void * running);

#endif
