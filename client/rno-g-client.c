#include "rno-g-client.h"

int send_cmd(char * raw_cmd) {
    int network_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    int connection_status = connect(network_socket, 
                                    (struct sockaddr *) &server_address, 
                                    sizeof(server_address));
    
    if (connection_status) {
        return connection_status;
    }

    write(network_socket, raw_cmd, strlen(raw_cmd));

    char ack[1024];
    read(network_socket, ack, sizeof(ack));

    printf("%s\n", ack);

    close(network_socket);

    return 0;
}

bool parse_args(char * raw_cmd, int num_args) {
    char * cmd_copy = (char *) malloc(sizeof(char) * (strlen(raw_cmd) + 1));
    strcpy(cmd_copy, raw_cmd);
    
    char * rest = NULL;
    char * cmd = strtok_r(cmd_copy, " ", &rest);

    bool valid = false;
    
    if (num_args == 0) {
        if (!strcmp(cmd, "LTE-ON")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-ON!")) {
            valid = true; 
        }
        else if (!strcmp(cmd, "LTE-OFF")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-OFF!")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-FACTORY-RESET")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-SOFT-RESET")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-HARD-RESET")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-POWER-CYCLE")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-STATE")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LTE-STATS")) {
            valid = true;
        }
        else if (!strcmp(cmd, "RADIANT-ON")) {
            valid = true;
        }
        else if (!strcmp(cmd, "RADIANT-OFF")) {
            valid = true;
        }
        else if (!strcmp(cmd, "J29-ON")) {
            valid = true;
        }
        else if (!strcmp(cmd, "J29-OFF")) {
            valid = true;
        }
        else if (!strcmp(cmd, "EXTBUS-ON")) {
            valid = true;
        }
        else if (!strcmp(cmd, "EXTBUS-OFF")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LOWTHRESH-ON")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LOWTHRESH-OFF")) {
            valid = true;
        }
        else if (!strcmp(cmd, "HEATER-ON")) {
            valid = true;
        }
        else if (!strcmp(cmd, "HEATER-OFF")) {
            valid = true;
        }
        else if (!strcmp(cmd, "EXPANDER-STATE")) {
            valid = true;
        }
        else if (!strcmp(cmd, "FAULT-STATE")) {
            valid = true;
        }
        else if (!strcmp(cmd, "MONITOR")) {
            valid = true;
        }
        else if (!strcmp(cmd, "B64MON")) {
            valid = true;
        }
        else if (!strcmp(cmd, "GET-BATT-MILLIVS")) {
            valid = true;
        }
        else if (!strcmp(cmd, "MODE-GET")) {
            valid = true;
        }
        else if (!strcmp(cmd, "I2C-DETECT")) {
            valid = true;
        }
        else if (!strcmp(cmd, "I2C-RESET")) {
            valid = true;
        }
        else if (!strcmp(cmd, "AM-I-BOOTLOADER")) {
            valid = true;
        }
        else if (!strcmp(cmd, "GET-STATION")) {
            valid = true;
        }
        else if (!strcmp(cmd, "GET-TIMESYNC-INTERVAL")) {
            valid = true;
        }
        else if (!strcmp(cmd, "FLUSH")) {
            valid = true;
        }
        else if (!strcmp(cmd, "LORA-SEND")) {
            valid = true;
        }
        else if (!strcmp(cmd, "NOW")) {
            valid = true;
        }
        else if (!strcmp(cmd, "VERSION")) {
            valid = true;
        }
        else if (!strcmp(cmd, "REV")) {
            valid = true;
        }
    }
    else if (num_args == 1) {
        if (!strcmp(cmd, "MONITOR-SCHED")) {
            valid = true;
        }
        else if (!strcmp(cmd, "I2C-UNSTICK")) {
            valid = true;
        }
        else if (!strcmp(cmd, "SYS-RESET")) {
            valid = true;
        }
        else if (!strcmp(cmd, "SET-STATION")) {
            valid = true;
        }
        else if (!strcmp(cmd, "SET-GPS-OFFSET")) {
            valid = true;
        }
        else if (!strcmp(cmd, "SET-TIMESYNC-INTERVAL")) {
            valid = true;
        }
    }
    else if (num_args == 2) {
        if (!strcmp(cmd, "AMPS-SET")) {
            long arg1 = strtol(strtok_r(NULL, " ", &rest), NULL, 16);
            long arg2 = strtol(strtok_r(NULL, " ", &rest), NULL, 16);
            valid = (arg1 >= 0x0 && arg1 <= 0x3f && 
                     arg2 >= 0x0 && arg2 <= 0x7);
        }
        else if (!strcmp(cmd, "SET-BATT-MILLIVS")) {
            valid = true;
        }
        else if (!strcmp(cmd, "I2C-READ")) {
            valid = true;
        }
    }
    else if (num_args == 3) {
        if (!strcmp(cmd, "I2C-WRITE")) {
            valid = true;
        }
    }

    free(cmd_copy);

    return valid;
}

int get_num_args(char * raw_cmd) {
    // make a copy of the raw command string since strtok_r will be used
    char * cmd_copy = (char *) malloc(sizeof(char) * (strlen(raw_cmd) + 1));
    strcpy(cmd_copy, raw_cmd);
    
    // set num_args to -1 to avoid counting the command itself
    int num_args = -1;
    char * rest = NULL;
    char * token = strtok_r(cmd_copy, " ", &rest);    
    
    while (token != NULL) {
        token = strtok_r(NULL, " ", &rest);
        num_args += 1;
    }
    
    free(cmd_copy);
    
    return num_args;
}

void parse_cmd(char * raw_cmd) {
    int num_args = get_num_args(raw_cmd);
    
    if (num_args >= 0) {
        int valid_args = parse_args(raw_cmd, num_args);

        if (valid_args) {
            send_cmd(raw_cmd);
        }
        else {
            fprintf(stderr, "INVALID COMMAND\n");
        }
    }
    else {
        fprintf(stderr, "INVALID NUMBER OF ARGS\n");
    }
}

int main() {
    size_t buf_size = 0; // initial value is 0 so getline will adjust it
    size_t len;
    int run = true;
    char * buf = NULL; // initial value is NULL so getline will malloc
    
    while(run) {
        printf("rno-g-shell > ");
        len = getline(&buf, &buf_size, stdin);
        buf[len - 1] = '\0';
        
        if (!strcmp(buf, "QUIT")) {
            run = false;
        }
        else {
            parse_cmd(buf);
        }

        free(buf);
        buf = NULL;
        buf_size = 0;
    }

    return 0;
}