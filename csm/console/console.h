#ifndef RNO_G_CONSOLE_H
#define RNO_G_CONSOLE_H

#define RNO_G_CONSOLE_BUF_SIZE 512

#include <arpa/inet.h>
#include <ctype.h>
#include <netinet/in.h>
#include <errno.h>
#include <signal.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

// data structure: console
typedef struct console console;

/*
 * function: console_create
 * ------------------------
 * create a new console instance
 *
 * addr: a string corresponding to the IP address to which the console connects
 *
 * port: a string corresponding to the port number to which the console connects
 *
 * run: a pointer to a run status variable to be used by the console
 *
 * sig_handler: a pointer to a signal handler function for the console
 *
 * returns: a pointer to a new console instance
 */
console * console_create(char * addr, char * port, uint8_t * run, void (* sig_handler) (int));


/*
 * function: console_destroy
 * ------------------------
 * destroys a console instance
 *
 * inst: a pointer to a console instance
 *
 * returns: 0 if the instance was successfully destroyed, a nonzero integer otherwise
 */
int console_destroy(console * inst);

/*
 * function: console_format_cmd
 * ----------------------------
 * format the buffer string of a console instance prior to sending it to the server
 *
 * inst: a pointer to a console instance
 *
 * returns: 0 if the buffer string was able to be properly formatted, a nonzero integer otherwise
 */
int console_format_cmd(console * inst);

/*
 * function: console_send_cmd
 * ---------------------------
 * send the buffer string of a console instance to the server
 *
 * inst: a pointer to a console instance
 *
 * returns: 0 if the buffer string was successfully sent and the acknowledgement was recieved, a nonzero integer otherwise
 */
int console_send_cmd(console * inst);

/*
 * function: console_run
 * ---------------------
 * initiate the run loop for a console instance
 *
 * inst: a pointer to a console instance
 *
 * returns: 0 if the run loop terminated successfully, a nonzero integer otherwise
 */
int console_run(console * inst);

#endif
