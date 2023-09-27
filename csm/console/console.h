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

typedef struct console console;

console * console_create(char * addr, char * port, uint8_t * run, void (* sig_handler) (int));

int console_destroy(console * inst);

int console_format_cmd(console * inst);

int console_send_cmd(console * inst);

int console_run(console * inst);

#endif
