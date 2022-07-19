#include "rno-g-server.h"

void print_cmd(char * cmd, char * client_address) {
    fputs("RECIEVED {", stdout);
    fputs(cmd, stdout);
    fputs("} FROM CLIENT AT {", stdout);
    fputs(client_address, stdout);
    fputs("}\n", stdout);
}

int main() {
    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));

    socklen_t client_addr_len;
    memset(&client_addr_len, 0, sizeof(socklen_t));
    
    char client_address[BUF_SIZE], cmd[BUF_SIZE];
    char ack[BUF_SIZE] = "Command Processed\n";

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

    while (1) {
        client_socket = accept(server_socket,
                               (struct sockaddr *) &client_addr,
                               &client_addr_len);
        
        inet_ntop(AF_INET, &client_addr, client_address, BUF_SIZE);
        
        len = read(client_socket, cmd, BUF_SIZE);
    
        print_cmd(cmd, client_address);

        write(client_socket, ack, BUF_SIZE);

        if (!strcmp(cmd, "LTE-OFF")) {
            break;
        }
    }
    
    close(server_socket);

    return 0;
}
