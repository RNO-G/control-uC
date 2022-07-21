#include <arpa/inet.h>
#include <pthread.h>

#include "rno-g-console.h"

#define CLIENT_LIM 10

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
 * function : print_cmd
 * --------------------
 * print the recieved command and the client address
 * 
 * cmd : the command string
 * 
 * client address: the IP address of the client
 * 
 * returns : nothing
 */
void print_cmd(char * cmd, char * client_address);
