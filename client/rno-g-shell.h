#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void parse_args(char * cmd, int num_args) {
    char * cmd_copy = (char *) malloc(sizeof(char) * (strlen(cmd) + 1));
    strcpy(cmd_copy, cmd);
    
    char * rest = NULL;
    char * pfx = strtok_r(cmd_copy, " ", &rest);
    
    if (!strcmp(pfx, "AMPS-SET")) {
        if (num_args != 2) {
            fprintf(stderr, "AMPS-SET : INVALID NUMBER OF ARGS\n");
        }
        else {
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
