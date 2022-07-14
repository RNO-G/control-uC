#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void send_cmd(char * cmd) {
}

void parse_args(char * cmd, int num_args) {
    char * cmd_copy = (char *) malloc(sizeof(char) * (strlen(cmd) + 1));
    strcpy(cmd_copy, cmd);
    
    char * rest = NULL;
    char * pfx = strtok_r(cmd_copy, " ", &rest);
    
    if (num_args == 0) {
        if (!strcmp(pfx, "LTE-ON")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-ON!")) {
            printf("%s\n", pfx); 
        }
        else if (!strcmp(pfx, "LTE-OFF")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-OFF!")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-FACTORY-RESET")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-SOFT-RESET")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-HARD-RESET")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-POWER-CYCLE")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-STATE")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LTE-STATS")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "RADIANT-ON")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "RADIANT-OFF")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "J29-ON")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "J29-OFF")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "EXTBUS-ON")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "EXTBUS-OFF")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LOWTHRESH-ON")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LOWTHRESH-OFF")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "HEATER-ON")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "HEATER-OFF")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "EXPANDER-STATE")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "FAULT-STATE")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "MONITOR")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "B64MON")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "GET-BATT-MILLIVS")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "MODE-GET")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "I2C-DETECT")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "I2C-RESET")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "AM-I-BOOTLOADER")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "GET-STATION")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "GET-TIMESYNC-INTERVAL")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "FLUSH")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "LORA-SEND")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "NOW")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "VERSION")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "REV")) {
            printf("%s\n", pfx);
        }
        else {
            fprintf(stderr, "INVALID COMMAND\n");
        }
    }
    else if (num_args == 1) {
        if (!strcmp(pfx, "MONITOR-SCHED")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "I2C-UNSTICK")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "SYS-RESET")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "SET-STATION")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "SET-GPS-OFFSET")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "SET-TIMESYNC-INTERVAL")) {
            printf("%s\n", pfx);
        }
        else {
            fprintf(stderr, "INVALID COMMAND\n");
        }
    }
    else if (num_args == 2) {
        if (!strcmp(pfx, "AMPS-SET")) {
            long arg1 = strtol(strtok_r(NULL, " ", &rest), NULL, 16);
            long arg2 = strtol(strtok_r(NULL, " ", &rest), NULL, 16);
            if (arg1 >= 0x0 && arg1 <= 0x3f && arg2 >= 0x0 && arg2 <= 0x7) {
                // printf("%x, %d\n", arg1, arg1);
                // printf("%x, %d\n", arg2, arg2);
                send_cmd(cmd);
            }
            else {
                fprintf(stderr, "AMPS-SET : INVALID ARG VALUES\n");
            }
        }
        else if (!strcmp(pfx, "SET-BATT-MILLIVS")) {
            printf("%s\n", pfx);
        }
        else if (!strcmp(pfx, "I2C-READ")) {
            printf("%s\n", pfx);
        }
        else {
            fprintf(stderr, "INVALID COMMAND\n");
        }
    }
    else if (num_args == 3) {
        if (!strcmp(pfx, "I2C-WRITE")) {
            printf("%s\n", pfx);
        }
        else {
            fprintf(stderr, "INVALID COMMAND\n");
        }
    }
    else {
        fprintf(stderr, "INVALID COMMAND\n");
    }

    free(cmd_copy);
}

int get_num_args(char * cmd) {
    char * cmd_copy = (char *) malloc(sizeof(char) * (strlen(cmd) + 1));
    strcpy(cmd_copy, cmd);
    
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

void parse_cmd(char * cmd) {
    int num_args = get_num_args(cmd);
    parse_args(cmd, num_args);
}
