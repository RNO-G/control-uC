#include "error.h"

void errno_check(int func_return, char * func_name) {
    if (func_return == -1) {
        char msg[BUF_SIZE];
        memset(msg, 0, BUF_SIZE);
        strerror_r(errno, msg, BUF_SIZE);

        fprintf(stderr, "%s : %s\n", func_name, msg);
        exit(EXIT_FAILURE);
    }
}

