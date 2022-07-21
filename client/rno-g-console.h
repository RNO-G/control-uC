#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#define BUF_SIZE 1024
#define PORT 1056

void error_check(int func_return) {
    if (func_return == -1) {
        perror("Error");
        exit(EXIT_FAILURE);
    }
}
