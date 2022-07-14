#include "rno-g-server.h"

int main() {
    char msg[1024];
    char ack[1024] = "Command Processed";

    int server_socket = socket(AF_INET, SOCK_STREAM, 0);

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    bind(server_socket, 
         (struct sockaddr *) &server_address, 
         sizeof(server_address));
    
    listen(server_socket, 10);

    int client_socket, len;
    while (true) {
        client_socket = accept(server_socket, NULL, NULL);
        len = read(client_socket, msg, sizeof(msg));
        msg[len] = '\0';
    
        printf("Recieved : {%s}\n", msg);
        printf("%d\n", strlen(msg));
        write(client_socket, ack, sizeof(ack));

        if (!strcmp(msg, "LTE-OFF")) {
            break;
        }
    }
    
    
    close(server_socket);

    return 0;
}
