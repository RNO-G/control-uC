#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_args(char * cmd, int num_args) {
    char * cmd_copy = (char *) malloc(sizeof(char) * (strlen(cmd) + 1));
    strcpy(cmd_copy, cmd);
    
    char * rest = NULL;
    char * pfx = strtok_r(cmd_copy, " ", &rest);
    
    if (num_args == 0) {
        if (!strcmp(pfx, "LTE-ON")) {

        }
        else if (!strcmp(pfx, "LTE-ON!")) {
            
        }
        else if (!strcmp(pfx, "LTE-OFF")) {
            
        }
        else if (!strcmp(pfx, "LTE-OFF!")) {
            
        }
        else if (!strcmp(pfx, "LTE-FACTORY-RESET")) {
            
        }
        else if (!strcmp(pfx, "LTE-SOFT-RESET")) {
            
        }
        else if (!strcmp(pfx, "LTE-HARD-RESET")) {
            
        }
        else if (!strcmp(pfx, "LTE-POWER-CYCLE")) {
            
        }
        else if (!strcmp(pfx, "LTE-STATE")) {
            
        }
        else if (!strcmp(pfx, "LTE-STATS")) {
            
        }
        else if (!strcmp(pfx, "RADIANT-ON")) {
            
        }
        else if (!strcmp(pfx, "RADIANT-OFF")) {
            
        }
        else if (!strcmp(pfx, "J29-ON")) {
            
        }
        else if (!strcmp(pfx, "J29-OFF")) {
            
        }
        else if (!strcmp(pfx, "EXTBUS-ON")) {
            
        }
        else if (!strcmp(pfx, "EXTBUS-OFF")) {
            
        }
        else if (!strcmp(pfx, "LOWTHRESH-ON")) {
            
        }
        else if (!strcmp(pfx, "LOWTHRESH-OFF")) {
            
        }
        else if (!strcmp(pfx, "HEATER-ON")) {
            
        }
        else if (!strcmp(pfx, "HEATER-OFF")) {
            
        }
        else if (!strcmp(pfx, "EXPANDER-STATE")) {
        
        }
        else if (!strcmp(pfx, "FAULT-STATE")) {
            
        }
        else if (!strcmp(pfx, "MONITOR")) {
            
        }
        else if (!strcmp(pfx, "B64MON")) {
            
        }
        else if (!strcmp(pfx, "GET-BATT-MILLIVS")) {
        
        }
        else if (!strcmp(pfx, "MODE-GET")) {
        
        }
        else if (!strcmp(pfx, "I2C-DETECT")) {
        
        }
        else if (!strcmp(pfx, "I2C-RESET")) {
            
        }
        else if (!strcmp(pfx, "AM-I-BOOTLOADER")) {
        
        }
        else if (!strcmp(pfx, "GET-STATION")) {
            
        }
        else if (!strcmp(pfx, "GET-TIMESYNC-INTERVAL")) {
            
        }
        else if (!strcmp(pfx, "FLUSH")) {
        
        }
        else if (!strcmp(pfx, "LORA-SEND")) {
            
        }
        else if (!strcmp(pfx, "NOW")) {
            
        }
        else if (!strcmp(pfx, "VERSION")) {
            
        }
        else if (!strcmp(pfx, "REV")) {
            
        }
        else {
            fprintf(stderr, "INVALID COMMAND\n");
        }
    }
    else if (num_args == 1) {
        if (!strcmp(pfx, "MONITOR-SCHED")) {
        
        }
        else if (!strcmp(pfx, "I2C-UNSTICK")) {
        
        }
        else if (!strcmp(pfx, "SYS-RESET")) {
        
        }
        else if (!strcmp(pfx, "SET-STATION")) {
            
        }
        else if (!strcmp(pfx, "SET-GPS-OFFSET")) {
            
        }
        else if (!strcmp(pfx, "SET-TIMESYNC-INTERVAL")) {
        
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
                printf("%x, %d\n", arg1, arg1);
                printf("%x, %d\n", arg2, arg2);
            }
            else {
                fprintf(stderr, "AMPS-SET : INVALID ARG VALUES\n");
            }
        }
        else if (!strcmp(pfx, "SET-BATT-MILLIVS")) {
        
        }
        else if (!strcmp(pfx, "I2C-READ")) {
        
        }
        else {
            fprintf(stderr, "INVALID COMMAND\n");
        }
    }
    else if (num_args == 3) {
        if (!strcmp(pfx, "I2C-WRITE")) {
        
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
