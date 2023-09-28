#include "console.h"

// run status variable
uint8_t run = 1;

// signal handler
void sig_handler(int sig) {
    if (sig == SIGINT) {
        run = 0;
    }
}

int main(int argc, char ** argv) {
    // the address and port must be provided as command line arguments
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    // create a console instance
    console * inst = console_create(argv[1], argv[2], &run, sig_handler);

    if (inst == NULL) {
        return EXIT_FAILURE;
    }

    // run the instance
    int ret = console_run(inst);

    // destroy the instance
    console_destroy(inst);

    return ret;
}

