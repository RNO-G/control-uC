#include "server.h"

int server_run_status = 1;
int num_connections_queued = 1;
int client_run_status[RNO_G_CLIENT_LIM] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int client_busy_status[RNO_G_CLIENT_LIM] = {1, 1, 1, 1, 1, 1, 1, 1, 1, 1};
int connection_queue[RNO_G_CLIENT_LIM] = {-1, -1, -1, -1, -1, -1, -1, -1, -1, -1};

pthread_mutex_t connection_queue_mutex;
pthread_mutex_t client_mutex[RNO_G_CLIENT_LIM];

pthread_t thread_pool[RNO_G_CLIENT_LIM];


// server signal handler
void server_sig_handler(int sig) {
    if (sig == SIGINT) {
        server_run_status = 0;
    }
}

// client signal handler
void client_sig_handler(int sig) {
    if (sig == SIGUSR1) {
        // get the id of the thread that calls this function
        pthread_t tid = pthread_self();

        for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
            // find the thread in the thread pool that matches this ID and tell it to exit
            if (pthread_equal(thread_pool[i], tid)) {
                if (pthread_mutex_lock(&(client_mutex[i]))) {
                    break;
                }

                client_run_status[i] = 0;
                client_busy_status[i] = 0;

                if (pthread_mutex_unlock(&(client_mutex[i]))) {
                    break;
                }

                break;
            }
        }
    }
}

// connection queue enqueue function
void connection_queue_enqueue(int socket_fd) {
    connection_queue[num_connections_queued] = socket_fd;
    num_connections_queued++;
}

// connection queue dequeue function
int connection_queue_dequeue() {
    int socket_fd = connection_queue[0];

    for (int i = 0; i < num_connections_queued - 1; i++) {
        connection_queue[i] = connection_queue[i + 1];
    }

    connection_queue[num_connections_queued] = -1;
    num_connections_queued--;

    return socket_fd;
}

int main(int argc, char ** argv) {
    // the server IP address, port number, and the uart path must be provided as command-line arguments
    if (argc != 4) {
        return EXIT_FAILURE;
    }

    // initialize the connection queue mutex (this may not need to be a global variable)
    if (pthread_mutex_init(&connection_queue_mutex, NULL)) {
        perror("server");
        return EXIT_FAILURE;
    }

    // initialize the client mutexes
    for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
        if (pthread_mutex_init(&(client_mutex[i]), NULL)) {
            perror("server");

            for (int j = 0; j < i; j++) {
                if (pthread_mutex_destroy(&(client_mutex[j]))) {
                    perror("server");
                }
            }

            if (pthread_mutex_destroy(&connection_queue_mutex)) {
                perror("server");
            }

            return EXIT_FAILURE;
        }
    }

    // create the server instance
    server * inst = server_create(argv[1], argv[2], argv[3],
                                  &server_run_status, client_run_status, client_busy_status,
                                  client_mutex, thread_pool, &num_connections_queued,
                                  &connection_queue_mutex, server_sig_handler, connection_queue_enqueue,
                                  client_sig_handler, connection_queue_dequeue);

    if (inst == NULL) {
        return EXIT_FAILURE;
    }

    // run the server instance
    server_run(inst);

    // destroy the server instance
    server_destroy(inst);

    // destroy the client mutexes
    for (int i = 0; i < RNO_G_CLIENT_LIM; i++) {
        if (pthread_mutex_destroy(&(client_mutex[i]))) {
            perror("server");

            for (int j = 0; j < i; j++) {
                if (pthread_mutex_destroy(&(client_mutex[j]))) {
                    perror("server");
                }
            }

            if (pthread_mutex_destroy(&connection_queue_mutex)) {
                perror("server");
            }

            return EXIT_FAILURE;
        }
    }

    // destroy the connection queue mutex
    if (pthread_mutex_destroy(&connection_queue_mutex)) {
        perror("server");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
