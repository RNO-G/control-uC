#include "rno-g-shell.h"

int main() {
    size_t buf_size = 0; // initial value is 0 so getline will adjust it
    size_t len;
    char * buf = NULL; // initial value is NULL so getline will malloc
    
    while(true) {
        printf("rno-g-shell > ");
        len = getline(&buf, &buf_size, stdin);
        
        if (!strcmp(buf, "quit\n")) {
            free(buf);
            buf = NULL;
            buf_size = 0;
            break;
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