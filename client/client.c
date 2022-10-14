#include "client.h"

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
        if (cmd[i] != ' ' && i > 0 && cmd[i - 1] == ' ') {
            num_args++;
        }
    }
    
    return num_args;
}

int parse_args(char * cmd, int num_args) {
    int valid = 0;
    
    if (num_args == 0) {
        if (!strcmp(cmd, "LTE-ON")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-ON!")) {
            valid = 1; 
        }
        else if (!strcmp(cmd, "LTE-OFF")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-OFF!")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-FACTORY-RESET")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-SOFT-RESET")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-HARD-RESET")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-POWER-CYCLE")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-STATE")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LTE-STATS")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "RADIANT-ON")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "RADIANT-OFF")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "J29-ON")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "J29-OFF")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "EXTBUS-ON")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "EXTBUS-OFF")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LOWTHRESH-ON")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LOWTHRESH-OFF")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "HEATER-ON")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "HEATER-OFF")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "EXPANDER-STATE")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "FAULT-STATE")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "MONITOR")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "B64MON")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "GET-BATT-MILLIVS")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "MODE-GET")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "I2C-DETECT")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "I2C-RESET")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "AM-I-BOOTLOADER")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "GET-STATION")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "GET-TIMESYNC-INTERVAL")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "FLUSH")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "LORA-SEND")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "NOW")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "VERSION")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "REV")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "DISCONNECT")) {
            valid = 1;
            running = 0;
        }
    }
    else if (num_args == 1) {
        
        char * arg = strchr(cmd, ' ') + 1;
        
        if (!strncmp(cmd, "MONITOR-SCHED", 13)) {
            valid = 1;
        }
        else if (!strncmp(cmd, "I2C-UNSTICK", 11)) {
            valid = 1;
        }
        else if (!strncmp(cmd, "SYS-RESET", 9)) {
            valid = 1;
        }
        else if (!strncmp(cmd, "SET-STATION", 11)) {
            valid = 1;
        }
        else if (!strncmp(cmd, "SET-GPS-OFFSET", 14)) {
            valid = 1;
        }
        else if (!strncmp(cmd, "SET-TIMESYNC-INTERVAL", 21)) {
            valid = 1;
        }
    }
    else if (num_args == 2) {
        char * arg1 = strchr(cmd, ' ') + 1;
        char * arg2 = strchr(arg1, ' ') + 1;
        
        int arg1_end;
        int arg1_len = strlen(arg1);
        for (int i = 0; i < arg1_len; i++) {
            if (arg1[i] == ' ') {
                arg1_end = i;
                break;
            }
        }

        if (!strncmp(cmd, "AMPS-SET", 8)) {
            long arg1_val = strtol(arg1, &arg1 + arg1_end, 16);
            long arg2_val = strtol(arg2, NULL, 16);
            valid = (arg1_val >= 0x0 && arg1_val <= 0x3f && 
                     arg2_val >= 0x0 && arg2_val <= 0x7);
        }
        else if (!strcmp(cmd, "SET-BATT-MILLIVS")) {
            valid = 1;
        }
        else if (!strcmp(cmd, "I2C-READ")) {
            valid = 1;
        }
    }
    else if (num_args == 3) {
        char * arg1 = strchr(cmd, ' ') + 1;
        char * arg2 = strchr(arg1, ' ') + 1;
        char * arg3 = strchr(arg2, ' ') + 1;
        
        int arg1_end;
        int arg1_len = strlen(arg1);
        for (int i = 0; i < arg1_len; i++) {
            if (arg1[i] == ' ') {
                arg1_end = i;
                break;
            }
        }
        
        int arg2_end;
        int arg2_len = strlen(arg2);
        for (int i = 0; i < arg2_len; i++) {
            if (arg2[i] == ' ') {
                arg2_end = i;
                break;
            }
        }

        if (!strcmp(cmd, "I2C-WRITE")) {
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
    char cmd[BUF_SIZE];
    char ack[BUF_SIZE];
    struct sigaction ign, sig;

    explicit_bzero(cmd, sizeof(char) * BUF_SIZE);
    explicit_bzero(ack, sizeof(char) * BUF_SIZE);
    
    explicit_bzero(&ign, sizeof(struct sigaction));
    explicit_bzero(&sig, sizeof(struct sigaction));
    
    ign.sa_flags = 0;
    ign.sa_handler = SIG_IGN;

    sig.sa_flags = 0;
    sig.sa_handler = sig_handler;

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
