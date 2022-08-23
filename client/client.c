#include "client.h"

int running = 1;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        running = 0;
        printf("\n");
    }
}

void connect_to_server(int * client_socket) {
    errno_check(*client_socket = socket(AF_INET, SOCK_STREAM, 0), "socket");
    
    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = INADDR_ANY;

    errno_check(connect(*client_socket, (struct sockaddr *) &server_address, sizeof(server_address)), "connect");
}

int format_cmd(char *cmd) {
    int len = strlen(cmd);
    
    if (len == 1) {
        return -1;
    }

    cmd[len - 1] = '\0';
    len--;

    int index = 0;

    for (int i = 0; i < len; i++) {
        if (!isspace(cmd[i]) || (i > 0 && !isspace(cmd[i - 1]))) {
            cmd[index] = cmd[i];
            index++;
        }
    }

    if (isspace(cmd[index - 1])) {
        cmd[index - 1] = '\0';
    }
    else {
        cmd[index] = '\0';
    }

    return 0;
}

int get_num_args(char * cmd) {
    char cmd_copy[BUF_SIZE];
    strcpy(cmd_copy, cmd);
    
    int num_args = -1;

    char * rest = NULL;
    char * token = strtok_r(cmd_copy, " ", &rest);    
    
    while (token != NULL) {
        token = strtok_r(NULL, " ", &rest);
        num_args += 1;
    }
    
    return num_args;
}

int parse_args(char * cmd, int num_args) {
    char cmd_copy[BUF_SIZE];
    strcpy(cmd_copy, cmd);
    
    char * rest = NULL;
    char * pfx = strtok_r(cmd_copy, " ", &rest);

    int valid = 0;
    
    if (num_args == 0) {
        if (!strcmp(pfx, "LTE-ON")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-ON!")) {
            valid = 1; 
        }
        else if (!strcmp(pfx, "LTE-OFF")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-OFF!")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-FACTORY-RESET")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-SOFT-RESET")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-HARD-RESET")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-POWER-CYCLE")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-STATE")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LTE-STATS")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "RADIANT-ON")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "RADIANT-OFF")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "J29-ON")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "J29-OFF")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "EXTBUS-ON")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "EXTBUS-OFF")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LOWTHRESH-ON")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LOWTHRESH-OFF")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "HEATER-ON")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "HEATER-OFF")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "EXPANDER-STATE")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "FAULT-STATE")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "MONITOR")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "B64MON")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "GET-BATT-MILLIVS")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "MODE-GET")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "I2C-DETECT")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "I2C-RESET")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "AM-I-BOOTLOADER")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "GET-STATION")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "GET-TIMESYNC-INTERVAL")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "FLUSH")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "LORA-SEND")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "NOW")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "VERSION")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "REV")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "DISCONNECT")) {
            valid = 1;
        }
    }
    else if (num_args == 1) {
        if (!strcmp(pfx, "MONITOR-SCHED")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "I2C-UNSTICK")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "SYS-RESET")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "SET-STATION")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "SET-GPS-OFFSET")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "SET-TIMESYNC-INTERVAL")) {
            valid = 1;
        }
    }
    else if (num_args == 2) {
        if (!strcmp(pfx, "AMPS-SET")) {
            long arg1 = strtol(strtok_r(NULL, " ", &rest), NULL, 16);
            long arg2 = strtol(strtok_r(NULL, " ", &rest), NULL, 16);
            valid = (arg1 >= 0x0 && arg1 <= 0x3f && 
                     arg2 >= 0x0 && arg2 <= 0x7);
        }
        else if (!strcmp(pfx, "SET-BATT-MILLIVS")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "I2C-READ")) {
            valid = 1;
        }
    }
    else if (num_args == 3) {
        if (!strcmp(pfx, "I2C-WRITE")) {
            valid = 1;
        }
    }

    return valid;
}

int send_cmd(int client_socket, char * cmd, char * ack) {
    if (write(client_socket, cmd, BUF_SIZE) < 1) {
        return -1;
    }
    
    if (read(client_socket, ack, BUF_SIZE) < 1) {
        return -1;
    }

    printf("%s\n", ack);

    return 0;
}

int main() {
    int client_socket;
    int num_args;
    char cmd[BUF_SIZE];
    char ack[BUF_SIZE];
    struct sigaction ign, sig;

    memset(cmd, 0, sizeof(char) * BUF_SIZE);
    memset(ack, 0, sizeof(char) * BUF_SIZE);
    
    memset(&ign, 0, sizeof(struct sigaction));
    memset(&sig, 0, sizeof(struct sigaction));
    
    ign.sa_flags = 0;
    ign.sa_handler = SIG_IGN;

    sig.sa_flags = 0;
    sig.sa_handler = signal_handler;

    errno_check(sigaction(SIGPIPE, &ign, NULL), "sigaction");
    errno_check(sigaction(SIGINT, &sig, NULL), "sigaction");

    connect_to_server(&client_socket);

    while (running) {
        printf("rno-g-shell > ");

        if (fgets(cmd, BUF_SIZE, stdin) == NULL) {
            printf("COULD NOT READ COMMAND\n");
            continue;
        }

        if (format_cmd(cmd) == -1) {
            printf("COMMAND LENGTH MUST BE GREATER THAN 0\n");
            continue;
        }

        if ((num_args = get_num_args(cmd)) == -1) {
            printf("INVALID NUMBER OF ARGUMENTS\n");
            continue;
        }

        if (!parse_args(cmd, num_args)) {
            printf("INVALID COMMAND\n");
            continue;
        }

        if (send_cmd(client_socket, cmd, ack) == -1) {
            printf("UNABLE TO SEND COMMAND TO SERVER\n");
            running = 0;
        }

        if (!strcmp(cmd, "DISCONNECT")) {
            running = 0;
        }
    }

    printf("DISCONNECTING CLIENT\n");

    errno_check(close(client_socket), "close");

    return EXIT_SUCCESS;
}
