#ifndef SERVER_H
#define SERVER_H

#include <arpa/inet.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
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
typedef struct cli_status cli_status;

/*
 * struct : cmd
 * ------------
 * a structure to contain a command string and the client
 * thread that issued the command
 *
 * cmd : the command string
 *
 * cli_tid : the client thread id
 */
typedef struct cmd cmd;

/*
 * function : main_signal_handler
 * ------------------------------
 * handle system signals recieved by the main thread
 * 
 * sig : the signal
 * 
 * returns : nothing
 */
void main_signal_handler(int sig);

/*
 * function : cmd_queue_signal_handler
 * -----------------------------------
 * handle system signals recieved by the command queue manager thread
 * 
 * sig : the signal
 * 
 * returns : nothing
 */
void cmd_queue_signal_handler(int sig);

/*
 * function : client_thread_signal_handler
 * ---------------------------------------
 * handle system signals recieved by a client thread
 * 
 * sig : the signal
 * 
 * returns : nothing
 */
void client_thread_signal_handler(int sig);

/*
 * function : client_queue_enqueue
 * -------------------------------
 * add a pending client connection to the client queue
 *
 * client_socket : the client socket
 *
 * returns : nothing
 */
void client_queue_enqueue(int client_socket);

/*
 * function : client_queue_dequeue
 * -------------------------------
 * remove the head client connection from the client queue
 *
 * returns : the client socket at the head of the queue
 */
int client_queue_dequeue();

/*
 * function : cmd_queue_enqueue
 * ----------------------------
 * add a pending command to the command queue
 *
 * cmd : the command string
 *
 * returns : nothing
 */
void cmd_queue_enqueue(char * cmd);

/*
 * function : cmd_queue_dequeue
 * ----------------------------
 * remove the head command from the command queue
 *
 * cmd : the pointer to store the command at the head of the queue
 *
 * returns : nothing
 */
void cmd_queue_dequeue(char * cmd);

/*
 * function : cmd_queue_manager
 * ----------------------------
 * thread handler function for managing the command queue
 *
 * returns : thread exit status
 */
void * cmd_queue_manager();

/*
 * function : manage_client
 * ------------------------
 * handler function for managing a client connection
 *
 * client_socket : the client connection
 *
 * returns : nothing
 */
void manage_client(int client_socket);

/*
 * function : manage_thread
 * ------------------------
 * thread handler function for managing the thread pool
 *
 * running : the pointer to the run status of the thread
 *
 * returns : thread exit status
 */
void * manage_thread(void * running);

#endif
