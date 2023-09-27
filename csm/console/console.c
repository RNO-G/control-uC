#include "console.h"

struct console {
    int socket;
    char buf[RNO_G_CONSOLE_BUF_SIZE+1];
    char ack[RNO_G_CONSOLE_BUF_SIZE+1];
    uint8_t * run;
    void (* sig_handler) (int);
};

console * console_create(char * addr, char * port, uint8_t * run, void (* sig_handler) (int)) {
    size_t addr_len = strlen(addr);

    if (addr_len < 7 && addr_len > 15) {
        fprintf(stderr, "console_create: invalid ip address length");
        return NULL;
    }

    size_t port_len = strlen(port);

    if (port_len < 4 && port_len > 5) {
        fprintf(stderr, "console_create: invalid port length");
        return NULL;
    }

    u_int8_t num_dots = 0;

    for (u_int8_t i = 0; i < addr_len; i++) {
        if (!(isdigit(addr[i]) || addr[i] == '.')) {
            fprintf(stderr, "console_create: invalid address");
            return NULL;
        }

        if (addr[i] == '.') {
            num_dots++;
        }
    }

    if (num_dots != 3) {
        fprintf(stderr, "console_create: invalid address");
        return NULL;
    }

    for (u_int8_t i = 0; i < port_len; i++) {
        if (!isdigit(port[i])) {
            fprintf(stderr, "console_create: invalid port");
            return NULL;
        }
    }

    struct sockaddr_in server_addr;

    if (inet_pton(AF_INET, addr, &server_addr.sin_addr) <= 0) {
        perror("console_create");
        return NULL;
    }

    unsigned long port_raw;
    errno = 0;

    if ((port_raw = strtoul(port, NULL, 10)) == 0 && errno != 0) {
        perror("console_create");
        return NULL;
    }

    if (port_raw < 1024 || port_raw > 65535) {
        fprintf(stderr, "console_create: invalid port");
        return NULL;
    }

    server_addr.sin_port = htons((uint16_t) port_raw);
    server_addr.sin_family = AF_INET;

    if (run == NULL) {
        fprintf(stderr, "console_create: run status pointer must not be null");
        return NULL;
    }

    if (sig_handler == NULL) {
        fprintf(stderr, "console_create: console signal handler function pointer must not be null");
    }

    int socket_fd;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("console_create");

        if (close(socket_fd)) {
            perror("console_create");
        }

        return NULL;
    }

    if (connect(socket_fd, (struct sockaddr *) &server_addr, sizeof(struct sockaddr_in))) {
        perror("console_create");

        if (close(socket_fd)) {
            perror("console_create");
        }

        return NULL;
    }

    console * inst = (console *) malloc(sizeof(console));

    if (inst == NULL) {
        perror("console_create");
        free(inst);

        if (close(socket_fd)) {
            perror("console_create");
        }

        return NULL;
    }

    memset(inst->buf, 0, RNO_G_CONSOLE_BUF_SIZE+1);
    memset(inst->ack, 0, RNO_G_CONSOLE_BUF_SIZE+1);

    inst->socket = socket_fd;
    inst->run = run;
    inst->sig_handler = sig_handler;

    return inst;
}

int console_destroy(console * inst) {
    if (close(inst->socket)) {
        perror("console_destroy");
        free(inst);
        return EXIT_FAILURE;
    }

    free(inst);
    return EXIT_SUCCESS;
}

int console_format_cmd(console * inst) {
    size_t len = strlen(inst->buf);
    
    if (len == 1) {
        return -1;
    }

    inst->buf[len - 1] = '\0';
    len--;

    int index = 0;

    for (size_t i = 0; i < len; i++) {
        if (!isspace(inst->buf[i]) && !isalnum(inst->buf[i]) && inst->buf[i] != '-') {
            return -2;
        }

        if (!isspace(inst->buf[i]) || (i > 0 && !isspace(inst->buf[i - 1]))) {
            inst->buf[index] = inst->buf[i];
            index++;
        }
    }

    if (isspace(inst->buf[index - 1])) {
        inst->buf[index - 1] = '\0';
    }
    else {
        inst->buf[index] = '\0';
    }

    return EXIT_SUCCESS;
}

int console_send_cmd(console * inst) {
    if (write(inst->socket, inst->buf, RNO_G_CONSOLE_BUF_SIZE+1) < 1) {
        return -1;
    }

    if (read(inst->socket, inst->ack, RNO_G_CONSOLE_BUF_SIZE+1) == -1) {
        return -2;
    }

    return EXIT_SUCCESS;
}

int console_run(console * inst) {
    struct sigaction ign = {.sa_flags = 0, .sa_handler = SIG_IGN};
    struct sigaction act = {.sa_flags = 0, .sa_handler = inst->sig_handler};

    if (sigaction(SIGPIPE, &ign, NULL)) {
        perror("console_run");
        return -1;
    }

    if (sigaction(SIGINT, &act, NULL)) {
        perror("console_run");
        return -2;
    }

    while (inst->run) {
        printf("rno-g-shell > ");

        if (fgets(inst->buf, RNO_G_CONSOLE_BUF_SIZE+1, stdin) == NULL) {
            printf("COULD NOT READ COMMAND\n");
            continue;
        }

        switch (console_format_cmd(inst)) {
            case -1:
                printf("COMMAND LENGTH MUST BE GREATER THAN 0\n");
                break;
            case -2:
                printf("COMMAND CONTAINS INVALID CHARACTERS\n");
                break;
            default:
                switch (console_send_cmd(inst)) {
                    case -1:
                        printf("UNABLE TO SEND COMMAND TO SERVER\n");
                        inst->run = 0;
                        break;
                    case -2:
                        printf("UNABLE TO RECIEVE ACKNOWLEDGEMENT FROM SERVER\n");
                        inst->run = 0;
                        break;
                    default:
                        printf("ACK: %s\n", inst->ack);
                }
        }
    }

    printf("DISCONNECTING CLIENT\n");

    return EXIT_SUCCESS;
}

