#include <netinet/in.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

/*
 * function : send_cmd
 * -------------------
 * send a request to run a command on the server
 * 
 * cmd : the command to be run
 * 
 * returns : 0 if the command was sent successfully, -1 otherwise
 */
int send_cmd(char * raw_cmd);

/*
 * function : parse_args
 * ---------------------
 * parse the arguments of the command before sending a request to the server
 * 
 * cmd : the command
 *
 * num_args : the number of arguments given to the command
 * 
 * returns : true if the command and args are valid, false if invalid
 */
bool parse_args(char * raw_cmd, int num_args);

/*
 * function : get_num_args
 * -----------------------
 * get the number of arguments provided to the command
 * 
 * raw_cmd : the raw command string
 * 
 * returns : the number of arguments given to the command, -1 if invalid
 */
int get_num_args(char * raw_cmd);

/*
 * function : parse_cmd
 * --------------------
 * parse command input from the user
 * 
 * raw_cmd : the raw command string given by the user
 * 
 * returns : nothing
 */
void parse_cmd(char * raw_cmd);
