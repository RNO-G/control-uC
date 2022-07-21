#include <ctype.h>

#include "rno-g-console.h"

/*
 * function : init_client
 * ----------------------
 * initialize the client network socket and connect to the server
 *
 * returns : 0 if the client successfully initialized and connected, 
 *           -1 otherwise
 */
int init_client();

/*
 * function : format_input
 * -----------------------
 * format raw input into proper command syntax by removing newline and
 * excessive whitespace
 * 
 * cmd : the command string
 * 
 * returns : 0 if the command was properly formatted, -1 if the raw input is
 *           invalid
 */
int format_input(char * cmd);

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
 * function : check_args
 * ---------------------
 * parse the arguments of the command before sending a request to the server
 * 
 * cmd : the command string
 *
 * num_args : the number of arguments given to the command
 * 
 * returns : 1 if the command and args are valid, 0 if invalid
 */
int check_args(char * cmd, int num_args);

/*
 * function : send_cmd
 * -------------------
 * send a request to run a command on the server
 * 
 * cmd : the command string
 * 
 * returns : 0 if the command was sent successfully, -1 otherwise
 */
int send_cmd(char * cmd);
