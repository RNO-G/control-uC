#include "rno-g-server.h"

struct cmd_node {
    char * cmd;
    struct cmd_node * next;
    struct cmd_node * prev;
};

int server_socket;
int queue_running = 1;
size_t num_clients = 0;
cmd_node * head = NULL;
cmd_node * tail = NULL;

void signal_handler(int sig) {
    if (sig == SIGINT) {
        error_check(close(server_socket));
        exit(EXIT_SUCCESS);
    }
}

int cmd_enqueue(char * cmd_str) {
    cmd_node * node = (cmd_node *) malloc(sizeof(cmd_node));

    node->cmd = (char *) malloc(sizeof(char) * BUF_SIZE);
    strcpy(node->cmd, cmd_str);
    
    if (head == NULL && tail == NULL) {
        head = node;
        node->next = NULL;
        node->prev = NULL;
    }
    else {
        tail->next = node;
        node->next = NULL;
        node->prev = tail;
    }

    tail = node;

    return 0;
}

int cmd_dequeue() {
    if (head == NULL && tail == NULL) {
        return -1;
    }

    cmd_node * tmp = head->next;

    free(head->cmd);
    free(head);

    if (tmp == NULL) {
        head = NULL;
        tail = NULL;        
    }
    else {
        head = tmp;
    }

    return 0;
}

void print_cmd(char * cmd, char * client_address) {
    fputs("RECIEVED {", stdout);
    fputs(cmd, stdout);
    fputs("} FROM CLIENT AT {", stdout);
    fputs(client_address, stdout);
    fputs("}\n", stdout);
}

void * manage_queue() {
    while (queue_running) {
        cmd_node * cur = head;

        if (cur) {
            printf("Scheduler is Processing {%s}\n", cur->cmd);

            if (!strcmp(cur->cmd, "LTE-OFF")) {
                while(cur) {
                    printf("Flushing Queue\n");
                    cmd_dequeue();
                    cur = head;
                }

                queue_running = 0;
                num_clients = 0;
            }
            else {
                if (!strcmp(cur->cmd, "DISCONNECT")) {
                    num_clients--;
                }

                cmd_dequeue();
            }  
        }

        sleep(1);
    }

    pthread_exit(EXIT_SUCCESS);
}

void * manage_client(void * client_socket_ptr) {
    int client_socket = *((int *) client_socket_ptr);
    // free(client_socket_ptr);

    char cmd[BUF_SIZE];
    char ack[BUF_SIZE] = "Command Recieved\n";
    
    while (1) {
        error_check(read(client_socket, cmd, BUF_SIZE));

        if (queue_running) {
            cmd_enqueue(cmd);
        }
        else {
            fputs("SERVER ASLEEP\n", stdout);
        }

        // print_cmd(cmd, client_address);

        error_check(write(client_socket, ack, BUF_SIZE));

        if (!strcmp(cmd, "DISCONNECT")) {
            break;
        }
    }

    error_check(close(client_socket));
    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char ** argv) {
    int client_socket;
    
    int * client_socket_ptr;
    
    pthread_t thread;
    pthread_attr_t attr;
    
    struct sockaddr_in server_addr;
    
    cmd_node * cur;

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    error_check(server_socket = socket(AF_INET, SOCK_STREAM, 0));
    error_check(bind(server_socket, (struct sockaddr *) &server_addr, sizeof(server_addr)));
    error_check(listen(server_socket, CLIENT_LIM));

    signal(SIGINT, signal_handler);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, 1);
    pthread_create(&thread, &attr, manage_queue, NULL);
    pthread_attr_destroy(&attr);

    while (1) {
        error_check(client_socket = accept(server_socket, (struct sockaddr *) NULL, NULL));
        
        // client_socket_ptr = (int *) malloc(sizeof(int));
        // *client_socket_ptr = client_socket;
        
        pthread_attr_init(&attr);
        pthread_attr_setdetachstate(&attr, 1);
        pthread_create(&thread, &attr, manage_client, &client_socket);
        pthread_attr_destroy(&attr);

        num_clients++;
    }
    
    error_check(close(server_socket));

    return EXIT_SUCCESS;
}
