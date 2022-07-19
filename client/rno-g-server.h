#include <arpa/inet.h>

#include "rno-g-console.h"

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
