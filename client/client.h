#ifndef CLIENT_H
#define CLIENT_H

#include <ctype.h>
#include <netinet/in.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include "error.h"

/*
 * function : signal_handler
 * -------------------------
 * handle system signals recieved by the client
 * 
 * sig : the signal
 * 
 * returns : nothing
 */
void signal_handler(int sig);

/*
 * function : connect
 * ------------------
 * initialize the client socket and connect it to the server
 *
 * client_socket : the client socket
 *
 * returns : nothing
 */
void connect_to_server(int * client_socket);

/*
 * function : format_cmd
 * ---------------------
 * format raw input into proper command syntax by removing newline and
 * excessive whitespace
 * 
 * cmd : the command string
 * 
 * returns : 0 if the command was properly formatted, -1 if the raw input is
 *           invalid
 */
int format_cmd(char * cmd);

/*
 * function : get_num_args
 * -----------------------
 * get the number of arguments provided to the command
 * 
 * cmd : the command string
 * 
 * returns : the number of arguments given to the command, -1 if invalid
 */
int get_num_args(char * cmd);

/*
 * function : parse_args
 * ---------------------
 * parse the arguments of the command before sending a request to the server
 * 
 * cmd : the command string
 *
 * num_args : the number of arguments given to the command
 * 
 * returns : 1 if the command and args are valid, 0 if invalid
 */
int parse_args(char * cmd, int num_args);

/*
 * function : send_cmd
 * -------------------
 * send a request to run a command on the server
 *
 * client_socket : the client socket
 *
 * cmd : the command string
 *
 * ack : the acknowledgement string
 * 
 * returns : 0 if the command was sent successfully, -1 otherwise
 */
int send_cmd(int client_socket, char * cmd, char * ack);

#endif
