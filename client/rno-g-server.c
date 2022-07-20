#include "rno-g-server.h"

void print_cmd(char * cmd, char * client_address) {
    fputs("RECIEVED {", stdout);
    fputs(cmd, stdout);
    fputs("} FROM CLIENT AT {", stdout);
    fputs(client_address, stdout);
    fputs("}\n", stdout);
}

int main() {
    pid_t child_pid;

    struct sockaddr_in client_addr;
    memset(&client_addr, 0, sizeof(struct sockaddr_in));

    socklen_t client_addr_len;
    memset(&client_addr_len, 0, sizeof(socklen_t));
    
    char client_address[BUF_SIZE], cmd[BUF_SIZE];
    char ack[BUF_SIZE] = "Command Processed\n";

    int server_socket, client_socket;

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fputs("UNABLE TO OPEN SERVER SOCKET\n", stderr);
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in server_address;
    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(9999);
    server_address.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        fputs("UNABLE TO BIND SERVER SOCKET TO SERVER IP ADDRESS\n", stderr);
        exit(EXIT_FAILURE);
    }
    
    if (listen(server_socket, 10) == 1) {
        fputs("UNABLE TO LISTEN TO SERVER SOCKET\n", stderr);
        exit(EXIT_FAILURE);
    }

    while (1) {
        if ((client_socket = accept(server_socket, (struct sockaddr *) &client_addr, &client_addr_len)) == -1) {
            fputs("UNABLE TO ESTABLISH CONNECTION WITH CLIENT\n", stderr);
            exit(EXIT_FAILURE);
        }

        if ((child_pid = fork()) == 0) {
            if (close(server_socket) == -1) {
                fputs("UNABLE TO CLOSE PARENT SERVER SOCKET\n", stderr);
                exit(EXIT_FAILURE);
            }

            inet_ntop(AF_INET, &client_addr, client_address, BUF_SIZE);

            while (1) {
                if (read(client_socket, cmd, BUF_SIZE) == -1) {
                    fputs("UNABLE TO READ DATA FROM CLIENT\n", stderr);
                    exit(EXIT_FAILURE);
                }
        
                print_cmd(cmd, client_address);

                if (write(client_socket, ack, BUF_SIZE) == -1) {
                    fputs("UNABLE TO WRITE DATA TO CLIENT\n", stderr);
                    exit(EXIT_FAILURE);
                }

                if (!strcmp(cmd, "DISCONNECT")) {
                    break;
                }
            }

            if (close(client_socket) == -1) {
                fputs("UNABLE TO CLOSE CONNECTION TO CLIENT\n", stderr);
                exit(EXIT_FAILURE);
            }

            exit(EXIT_SUCCESS);
        }
        else if (child_pid == -1) {
            fputs("UNABLE TO FORK FROM PARENT PROCESS\n", stderr);
            exit(EXIT_FAILURE);
        }
        else {
            if (close(client_socket) == -1) {
                fputs("UNABLE TO CLOSE CONNECTION TO CLIENT\n", stderr);
                exit(EXIT_FAILURE);
            }
        }
    }
    

    if (close(server_socket) == -1) {
        fputs("UNABLE TO CLOSE SERVER SOCKET\n", stderr);
        exit(EXIT_FAILURE);
    }

    return EXIT_SUCCESS;
}
