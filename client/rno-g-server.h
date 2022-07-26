#include <arpa/inet.h>
#include <pthread.h>

#include "rno-g-console.h"

#define CLI_LIM 10
#define CMD_LIM 32

typedef struct cmd_node cmd_node;

/*
 * function : signal_handler
 * -------------------------
 * handle system signal recieved by the client
 * 
 * sig : the signal
 * 
 * returns : nothing
 */
void signal_handler(int sig);

/*
 * function : cmd_enqueue
 * ----------------------
 * add a command to the queue
 * 
 * cmd : the command string
 * 
 * returns : 0 if the command was successfully added, -1 if the queue is full
 */
int cmd_enqueue(char * cmd);

/*
 * function : cmd_dequeue
 * ----------------------
 * remove a command from the front of the queue
 * 
 * returns : 0 if the command was successfully removed, -1 if the queue is
 *           already empty
 */
int cmd_dequeue();

/*
 * function : cmd_flush
 * --------------------
 * flush the command queue
 * 
 * returns : 0 if the queue was successfully emptied
 */
int cmd_flush();

/*
 * function : print_cmd_queue
 * --------------------------
 * print the command queue
 * 
 * returns : nothing
 */
void print_cmd_queue();

/*
 * function : manage_cmd_queue
 * ---------------------------
 * thread function to manage the command queue
 * 
 * returns : nothing
 */
void * manage_cmd_queue();

/*
 * function : manage_client
 * ------------------------
 * thread function to manage a client connection to the server
 * 
 * client_socket_ptr : the pointer corresponding to the client socket
 * 
 * returns : nothing
 */
void * manage_client(void * client_socket_ptr);
