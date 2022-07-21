#include <arpa/inet.h>
#include <pthread.h>

#include "rno-g-console.h"

#define CLIENT_LIM 10
#define PORT 9999

typedef struct cmd_node cmd_node;

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
