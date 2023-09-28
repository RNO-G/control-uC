#include "client.h"
#include "constants.h"

int running = 1;

void sig_handler(int sig) {
    if (sig == SIGINT) {
        running = 0;
        printf("\n");
    }
}

void connect_to_svr(int * cli_sock) {
    errno_check(*cli_sock = socket(AF_INET, SOCK_STREAM, 0), "socket");
    
    struct sockaddr_in svr_address;
    svr_address.sin_family = AF_INET;
    svr_address.sin_port = htons(PORT);
    svr_address.sin_addr.s_addr = INADDR_ANY;

    errno_check(connect(*cli_sock, (struct sockaddr *) &svr_address, sizeof(svr_address)), "connect");
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
    int num_args = 0;
    int len = strlen(cmd);
    
    for (int i = 0; i < len; i++) {
        if (cmd[i] == ' ') {
            num_args++;
        }
    }

    return num_args;
}

int parse_args(char * cmd, int num_args) {
    char cmd_copy[BUF_SIZE];
    
    int valid = 0;
    
    strcpy(cmd_copy, cmd);
    char * rest = NULL;
    char * pfx = strtok_r(cmd_copy, " ", &rest);
    
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
            running = 0;
        }
    }
    else if (num_args == 1) {

        char * arg = strtok_r(NULL, " ", &rest);
        
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

        char * arg1 = strtok_r(NULL, " ", &rest);
        char * arg2 = strtok_r(NULL, " ", &rest);

        if (!strcmp(pfx, "AMPS-SET")) {
            long arg1_val = strtol(arg1, NULL, 16);
            long arg2_val = strtol(arg2, NULL, 16);

            valid = (arg1_val >= 0x0 && arg1_val <= 0x3f && 
                     arg2_val >= 0x0 && arg2_val <= 0x7);
        }
        else if (!strcmp(pfx, "SET-BATT-MILLIVS")) {
            valid = 1;
        }
        else if (!strcmp(pfx, "I2C-READ")) {
            valid = 1;
        }
    }
    else if (num_args == 3) {

        char * arg1 = strtok_r(NULL, " ", &rest);
        char * arg2 = strtok_r(NULL, " ", &rest);
        char * arg3 = strtok_r(NULL, " ", &rest);
        
        if (!strcmp(pfx, "I2C-WRITE")) {
            valid = 1;
        }
    }

    return valid;
}

int send_cmd(int cli_sock, char * cmd, char * ack) {
    if (write(cli_sock, cmd, BUF_SIZE) < 1) {
        return -1;
    }
    
    if (read(cli_sock, ack, BUF_SIZE) < 1) {
        return -1;
    }

    printf("%s\n", ack);

    return 0;
}

int main() {
    int cli_sock;
    int num_args;

    char cmd[BUF_SIZE] = {'\0'};
    char ack[BUF_SIZE] = {'\0'};
    
    struct sigaction ign = {.sa_flags = 0, .sa_handler = SIG_IGN};
    struct sigaction sig = {.sa_flags = 0, .sa_handler = sig_handler};

    errno_check(sigaction(SIGPIPE, &ign, NULL), "sigaction");
    errno_check(sigaction(SIGINT, &sig, NULL), "sigaction");

    connect_to_svr(&cli_sock);

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

        if (send_cmd(cli_sock, cmd, ack) == -1) {
            printf("UNABLE TO SEND COMMAND TO svr\n");
            running = 0;
        }
    }

    printf("DISCONNECTING CLIENT\n");

    errno_check(close(cli_sock), "close");

    return EXIT_SUCCESS;
}
