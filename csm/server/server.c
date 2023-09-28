#include "server.h"

struct server {
    // these are created by the server (need to be freed)
    int socket;
    int uart;
    pthread_cond_t client_cond[RNO_G_CLIENT_LIM];
    pthread_mutex_t * uart_mutex;
    client * clients[RNO_G_CLIENT_LIM];

    // these are pointers passed to the server
    int * run;
    int * client_run;
    int * client_busy;
    pthread_mutex_t * client_mutex;
    pthread_t * thread_pool;

    int * num_connections_queued;
    pthread_mutex_t * connection_queue_mutex;
    void (* sig_handler) (int);
    void (* connection_queue_enqueue) (int);
};

server * server_create(char * addr, char * port, char * uart_path, int * run,
                       int client_run[RNO_G_CLIENT_LIM], int client_busy[RNO_G_CLIENT_LIM],
                       pthread_mutex_t client_mutex[RNO_G_CLIENT_LIM], pthread_t thread_pool[RNO_G_CLIENT_LIM],
                       int * num_connections_queued, pthread_mutex_t * connection_queue_mutex,
                       void (* server_sig_handler) (int), void (* connection_queue_enqueue) (int),
                       void (* client_sig_handler) (int), int (* connection_queue_dequeue) ()) {

    // check if the address string is of proper length
    size_t addr_len = strlen(addr);

    if (addr_len < 7 && addr_len > 15) {
        fprintf(stderr, "server_create : invalid ip address length");
        return NULL;
    }

    // check if the port string is of proper length
    size_t port_len = strlen(port);

    if (port_len < 4 && port_len > 5) {
        fprintf(stderr, "server_create : invalid port length");
        return NULL;
    }

    // check if the address string is properly formatted
    u_int8_t num_dots = 0;

    for (u_int8_t i = 0; i < addr_len; i++) {
        if (!(isdigit(addr[i]) || addr[i] == '.')) {
            fprintf(stderr, "server_create : invalid address");
            return NULL;
        }

        if (addr[i] == '.') {
            num_dots++;
        }
    }

    if (num_dots != 3) {
        fprintf(stderr, "server_create : invalid address");
        return NULL;
    }

    // check if the port string is properly formatted
    for (u_int8_t i = 0; i < port_len; i++) {
        if (!isdigit(port[i])) {
            fprintf(stderr, "server_create : invalid port");
            return NULL;
        }
    }

    // check if the uart path string corresponds to valid UNIX file
    if (access(uart_path, F_OK)) {
        perror("server_create");
        return NULL;
    }

    // initialize a sockaddr_in struct to store the address
    struct sockaddr_in server_addr;

    if (inet_pton(AF_INET, addr, &server_addr.sin_addr) <= 0) {
        perror("server_create");
        return NULL;
    }

    unsigned long port_raw;
    errno = 0;

    // check if the port string can be converted to a numeric type and check if it falls within the valid port range
    if ((port_raw = strtoul(port, NULL, 10)) == 0 && errno != 0) {
        perror("server_create");
        return NULL;
    }

    if (port_raw < 1024 || port_raw > 65535) {
        fprintf(stderr, "server_create : invalid port");
        return NULL;
    }

    // set the port of the sockaddr_in struct
    server_addr.sin_port = htons((uint16_t) port_raw);
    server_addr.sin_family = AF_INET;

    // check if the variable or function pointers are NULL
    if (run == NULL) {
        fprintf(stderr, "server_create: server run status pointer must not be null");
    }

    if (client_run == NULL) {
        fprintf(stderr, "server_create: run status pointer array must not be null");
        return NULL;
    }

    if (client_busy == NULL) {
        fprintf(stderr, "server_create : busy status pointer array must not be null");
        return NULL;
    }

    if (client_mutex == NULL) {
        fprintf(stderr, "server_create : client mutex pointer array must not be null");
        return NULL;
    }

    if (num_connections_queued == NULL) {
        fprintf(stderr, "server_create: number of queued connections pointer must not be null");
        return NULL;
    }

    if (connection_queue_mutex == NULL) {
        fprintf(stderr, "server_create: connection queue mutex must not be null");
        return NULL;
    }

    if (server_sig_handler == NULL) {
        fprintf(stderr, "server_create: server signal handler function must not be null");
        return NULL;
    }

    if (connection_queue_enqueue == NULL) {
        fprintf(stderr, "server_create: connection queue enqueue function must not be null");
        return NULL;
    }

    if (client_sig_handler == NULL) {
        fprintf(stderr, "server_create: client signal handler function must not be null");
        return NULL;
    }

    if (connection_queue_dequeue == NULL) {
        fprintf(stderr, "server_create: connection queue dequeue function must not be null");
        return NULL;
    }

    // open the uart file
    int uart;

    if ((uart = open(uart_path, O_RDWR)) == -1) {
        perror("server_create");

        if (close(uart)) {
            perror("server_create");
        }

        return NULL;
    }

    // do we need an flock on the uart?

    // declare and initialize a mutex for the uart
    pthread_mutex_t uart_mutex;

    if (pthread_mutex_init(&uart_mutex, NULL)) {
        perror("server_create");

        if (close(uart)) {
            perror("server_create");
        }

        return NULL;
    }

    // check if the thread pool is NULL
    if (thread_pool == NULL) {
        fprintf(stderr, "server_create: thread pool pointer array must not be null");

        if (close(uart)) {
            perror("server_create");
        }

        if (pthread_mutex_destroy(&uart_mutex)) {
            perror("server_create");
        }

        return NULL;
    }

    // create the server socket and bind the socket to the server's IP address
    int socket_fd;

    if ((socket_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("console_create");

        if (close(uart)) {
            perror("server_create");
        }

        if (pthread_mutex_destroy(&uart_mutex)) {
            perror("server_create");
        }

        if (close(socket_fd)) {
            perror("console_create");
        }

        return NULL;
    }

    if (bind(socket_fd, (struct sockaddr *) &server_addr, sizeof(server_addr))) {
        perror("console_create");

        if (close(uart)) {
            perror("server_create");
        }

        if (pthread_mutex_destroy(&uart_mutex)) {
            perror("server_create");
        }

        if (close(socket_fd)) {
            perror("console_create");
        }

        return NULL;
    }

    // set the socket to listen for connections
    if (listen(socket_fd, 0)) {
        perror("console_create");

        if (close(uart)) {
            perror("server_create");
        }

        if (pthread_mutex_destroy(&uart_mutex)) {
            perror("server_create");
        }

        if (close(socket_fd)) {
            perror("console_create");
        }

        return NULL;
    }

    // malloc a server instance and initialize the struct members
    server * inst = (server *) malloc(sizeof(server));

    if (inst == NULL) {
        if (close(uart)) {
            perror("server_create");
        }

        if (pthread_mutex_destroy(&uart_mutex)) {
            perror("server_create");
        }

        if (close(socket_fd)) {
            perror("console_create");
        }

        free(inst);

        return NULL;
    }

    // create the pthread_cond_t array
    for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
        if (pthread_cond_init(&(inst->client_cond[i]), NULL)) {
            perror("server_create");

            for (int j = 0; j < i; j++) {
                if (pthread_cond_destroy(&(inst->client_cond[j]))) {
                    perror("server_create");
                }
            }

            if (close(uart)) {
                perror("server_create");
            }

            if (pthread_mutex_destroy(&uart_mutex)) {
                perror("server_create");
            }

            return NULL;
        }
    }

    // create the client data structures
    for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
        inst->clients[i] = client_create(uart, &(client_run[i]), &(client_busy[i]), num_connections_queued,
                                         &(inst->client_cond[i]), &(client_mutex[i]), connection_queue_mutex,
                                         &uart_mutex, client_sig_handler, connection_queue_dequeue);

        if (inst->clients[i] == NULL) {
            for (int j = 0; j < i; j++) {
                client_destroy(inst->clients[j]);

                if (pthread_cond_destroy(&(inst->client_cond[j]))) {
                    perror("server_create");
                }
            }

            for (int j = i; j < RNO_G_CLIENT_LIM; j++) {
                if (pthread_cond_destroy(&(inst->client_cond[j]))) {
                    perror("server_create");
                }
            }

            if (close(uart)) {
                perror("server_create");
            }

            if (pthread_mutex_destroy(&uart_mutex)) {
                perror("server_create");
            }

            if (close(socket_fd)) {
                perror("console_create");
            }

            free(inst);

            return NULL;
        }
    }

    inst->socket = socket_fd;
    inst->uart = uart;
    inst->uart_mutex = &uart_mutex;
    inst->run = run;

    inst->client_run = client_run;
    inst->client_busy = client_busy;
    inst->client_mutex = client_mutex;
    inst->thread_pool = thread_pool;
    inst->num_connections_queued = num_connections_queued;
    inst->connection_queue_mutex = connection_queue_mutex;
    inst->sig_handler = server_sig_handler;
    inst->connection_queue_enqueue = connection_queue_enqueue;

    return inst;
}

int server_destroy(server * inst) {
    // close the server socket
    if (close(inst->socket)) {
        perror("server_destroy");

        if (close(inst->uart)) {
            perror("server_destroy");
        }

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            if (pthread_cond_destroy(&(inst->client_cond[i]))) {
                perror("server_destroy");
            }
        }

        return EXIT_FAILURE;
    }

    // close the uart
    if (close(inst->uart)) {
        perror("server_destroy");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            if (pthread_cond_destroy(&(inst->client_cond[i]))) {
                perror("server_destroy");
            }
        }

        return EXIT_FAILURE;
    }

    // destroy the clients and the pthread_cond_t array
    for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
        client_destroy(inst->clients[i]);

        if (pthread_cond_destroy(&(inst->client_cond[i]))) {
            perror("server_destroy");

            for (int j = i; j < RNO_G_CLIENT_LIM; j++) {
                if (pthread_cond_destroy(&(inst->client_cond[i]))) {
                    perror("server_destroy");
                }

                client_destroy(inst->clients[j]);
            }

            return EXIT_FAILURE;
        }
    }

    free(inst);

    return EXIT_SUCCESS;
}

int server_run(server * inst) {
    // create a sigmask for the threads
    struct sigaction ign = {.sa_flags = 0, .sa_handler = SIG_IGN};
    struct sigaction sig = {.sa_flags = 0, .sa_handler = inst->sig_handler};
    sigset_t set;

    // create an empty sigmask
    if (sigemptyset(&set)) {
        perror("server_run");
        return EXIT_FAILURE;
    }

    // block SIGINT and SIGPIPE for the clients
    if (sigaddset(&set, SIGINT)) {
        perror("server_run");
        return EXIT_FAILURE;
    }

    if (sigaddset(&set, SIGPIPE)) {
        perror("server_run");
        return EXIT_FAILURE;
    }

    if (pthread_sigmask(SIG_BLOCK, &set, NULL)) {
        perror("server_run");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
        // lock the client mutex
        if (pthread_mutex_lock(&(inst->client_mutex[i]))) {
            perror("server_run");
            return EXIT_FAILURE;
        }

        // set the client to run, but not to be busy
        inst->client_run[i] = 1;
        inst->client_busy[i] = 0;

        // unlock the client mutex
        if (pthread_mutex_unlock(&(inst->client_mutex[i]))) {
            perror("server_run");
            return EXIT_FAILURE;
        }

        // initialize the threads and supply them with the client_run function and the client pointers
        if (pthread_create(&(inst->thread_pool[i]), NULL, client_run, (void *) inst->clients[i])) {
            for (int j = 0; j < i; j++) {
                pthread_kill(inst->thread_pool[j], SIGUSR1);
                pthread_join(inst->thread_pool[j], NULL);
            }

            return EXIT_FAILURE;
        }
    }

    // re-empty the signal mask and set the server to unblock SIGINT and SIGPIPE
    if (sigemptyset(&set)) {
        perror("server_run");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            pthread_kill(inst->thread_pool[i], SIGUSR1);
            pthread_join(inst->thread_pool[i], NULL);
        }

        return EXIT_FAILURE;
    }

    if (sigaddset(&set, SIGINT)) {
        perror("server_run");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            pthread_kill(inst->thread_pool[i], SIGUSR1);
            pthread_join(inst->thread_pool[i], NULL);
        }

        return EXIT_FAILURE;
    }

    if (sigaddset(&set, SIGPIPE)) {
        perror("server_run");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            pthread_kill(inst->thread_pool[i], SIGUSR1);
            pthread_join(inst->thread_pool[i], NULL);
        }

        return EXIT_FAILURE;
    }

    if (pthread_sigmask(SIG_UNBLOCK, &set, NULL)) {
        perror("server_run");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            pthread_kill(inst->thread_pool[i], SIGUSR1);
            pthread_join(inst->thread_pool[i], NULL);
        }

        return EXIT_FAILURE;
    }

    // the server should ignore SIGPIPE
    if (sigaction(SIGPIPE, &ign, NULL)) {
        perror("server_run");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            pthread_kill(inst->thread_pool[i], SIGUSR1);
            pthread_join(inst->thread_pool[i], NULL);
        }

        return EXIT_FAILURE;
    }

    // the server should ignore SIGUSR1
    if (sigaction(SIGUSR1, &ign, NULL)) {
        perror("server_run");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            pthread_kill(inst->thread_pool[i], SIGUSR1);
            pthread_join(inst->thread_pool[i], NULL);
        }

        return EXIT_FAILURE;
    }

    // the server should NOT ignore SIGINT
    if (sigaction(SIGINT, &sig, NULL)) {
        perror("server_run");

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            pthread_kill(inst->thread_pool[i], SIGUSR1);
            pthread_join(inst->thread_pool[i], NULL);
        }

        return EXIT_FAILURE;
    }

    int client_socket;

    while (1) {
        // wait for a connection request, break if that fails
        if ((client_socket = accept(inst->socket, NULL, NULL)) == -1) {
            break;
        }

        printf("socket recieved by server\n");

        // lock the connection queue mutex
        if (pthread_mutex_lock(inst->connection_queue_mutex)) {
            perror("server_run");
            break;
        }

        // break if the number of queued connections is already at maximum (this might need to change; we may not want the server to exit in this case)
        // add the connection to the queue otherwise
        if (*(inst->num_connections_queued) == RNO_G_CLIENT_LIM) {
            if (close(client_socket)) {
                perror("server_run");
                break;
            }
        }
        else {
            printf("socket added to queue\n");
            inst->connection_queue_enqueue(client_socket);
        }

        // unlock the connection queue mutex
        if (pthread_mutex_unlock(inst->connection_queue_mutex)) {
            perror("server_run");
            break;
        }

        // check if any of the clients are not busy (and therefore sleeping) so they can handle the new connection
        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            // lock the client mutex
            if (pthread_mutex_lock(&(inst->client_mutex[i]))) {
                perror("server_run");
                break;
            }

            // find the first non-busy client and signal it to wake up; unlock the client mutex
            if (inst->client_busy[i] == 0) {
                if (pthread_cond_signal(&(inst->client_cond[i]))) {
                    perror("server_run");

                    if (pthread_mutex_unlock(&(inst->client_mutex[i]))) {
                        perror("server_run");
                        break;
                    }

                    break;
                }
            }

            if (pthread_mutex_unlock(&(inst->client_mutex[i]))) {
                perror("server_run");
                break;
            }
        }

        // break the loop if the server should no longer be running
        if (!*(inst->run)) {
            break;
        }
    }

    // signal each client thread to exit
    for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
        pthread_kill(inst->thread_pool[i], SIGUSR1);
        pthread_cond_signal(&(inst->client_cond[i]));
        pthread_join(inst->thread_pool[i], NULL);
    }

    return EXIT_SUCCESS;
}
