#include "rno-g-client.h"

int init_client(int * network_socket) {
    *network_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    return connect(*network_socket,
                   (struct sockaddr *) &server_address,
                   sizeof(server_address));
}

int format_input(char * cmd) {
    size_t len = strlen(cmd);
    
    if (len == 1) {
        return -1;
    }

    cmd[len - 1] = '\0';

    return 0;
}

int get_num_args(char * cmd) {
    
    // make a copy of the raw command string since strtok_r will be used
    char cmd_copy[BUF_SIZE];
    strcpy(cmd_copy, cmd);
    
    // set num_args to -1 to avoid counting the command itself
    int num_args = -1;

    char * rest = NULL;
    char * token = strtok_r(cmd_copy, " ", &rest);    
    
    while (token != NULL) {
        token = strtok_r(NULL, " ", &rest);
        num_args += 1;
    }
    
    return num_args;
}

int check_args(char * cmd, int num_args) {
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

int send_cmd(char * cmd, char * ack, int network_socket) {
    if (write(network_socket, cmd, BUF_SIZE) == -1) {
        return -1;
    }
    
    if (read(network_socket, ack, BUF_SIZE) == -1) {
        return -1;
    }

    fputs(ack, stdout);

    return 0;
}

int main(int argc, char ** argv) {
    char cmd[BUF_SIZE], ack[BUF_SIZE];
    memset(cmd, 0, sizeof(char) * BUF_SIZE);
    
    int num_args;
    int network_socket;

    if (init_client(&network_socket) != 0) {
        fputs("UNABLE TO CONNECT TO SERVER\n", stderr);
        exit(EXIT_FAILURE);
    }

    while (1) {
        fputs("rno-g-shell > ", stdout);

        if (fgets(cmd, BUF_SIZE, stdin) == NULL) {
            fputs("COULD NOT READ COMMAND\n", stderr);
            continue;
        }

        if (format_input(cmd) == -1) {
            fputs("COMMAND LENGTH MUST BE GREATER THAN 0\n", stderr);
            continue;
        }

        if (!strcmp(cmd, "QUIT")) {
            break;
        }

        if ((num_args = get_num_args(cmd)) == -1) {
            fputs("INVALID NUMBER OF ARGUMENTS\n", stderr);
            continue;
        }

        if (!check_args(cmd, num_args)) {
            fputs("INVALID COMMAND\n", stderr);
            continue;
        }

        if (send_cmd(cmd, ack, network_socket) == -1) {
            fputs("UNABLE TO SEND COMMAND TO SERVER\n", stderr);
            continue;
        }
    }

    if (close(network_socket) == -1) {
        fputs("UNABLE TO CLOSE CONNECTION TO SERVER\n", stderr);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}