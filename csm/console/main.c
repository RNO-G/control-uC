#include "console.h"

uint8_t run = 1;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        run = 0;
    }
}

int main(int argc, char ** argv) {
    if (argc != 3) {
        return EXIT_FAILURE;
    }

    console * inst = console_create(argv[1], argv[2], &run, sig_handler);

    if (inst == NULL) {
        return EXIT_FAILURE;
    }

    int ret = console_run(inst);
    console_destroy(inst);

    return ret;
}

