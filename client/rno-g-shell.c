#include "rno-g-shell.h"

int main() {
    size_t buf_size = 512;
    size_t len;
    char * buf = (char *) malloc(sizeof(char) * buf_size);
    
    while(true) {
        printf("shell > ");
        len = getline(&buf, &buf_size, stdin);
        printf("%s", buf);
        printf("%d\n", strlen(buf));
        if (!strcmp(buf, "quit\n")) {
            break;
        }
    }

    free(buf);
    return 0;
}