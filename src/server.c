#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>

void handle_client(int client_fd);
void reap_children(int signo);
void* thread_main(void* arg);

//main server function
void server_start(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd <0) {
        perror("Socket creation failed");
        exit(1);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_fd, (struct sockaddr*)&addr, sizeof(addr))<0) {
        perror("bind error");
        exit(1);
    }

    if (listen(server_fd, 1) <0) {
        perror("Listening failed");
        exit(1);
    }

    printf("Listening on port %d...\n", port);

    struct sigaction sa;
    sa.sa_handler = reap_children;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    sigaction(SIGCHLD, &sa, NULL);

    while (1) {

        int client_fd = accept(server_fd, NULL, NULL);

        if (client_fd <0) {
            perror("Accept failed");
            exit(1);
        }

		//Using threads fir handling clients instead of fork()
		int* client_fd_ptr = malloc(sizeof(int));
		*client_fd_ptr = client_fd;

		pthread_t tid;
		pthread_create(&tid, NULL, thread_main, client_fd_ptr);
		pthread_detach(tid);
    }
}

//Client handling function
void handle_client(int client_fd) {

    char buffer[1024];

    printf("PID=%d Client connected.\n", getpid());

    while (1) {
        ssize_t bytes_read = read(client_fd, buffer, sizeof(buffer));

        if (bytes_read == 0) {
            printf("client disconnected gracefully.\n");
            break;
        }

        if (bytes_read < 0) {
            perror("read failed");
            break;
        }

        write(client_fd, buffer, bytes_read);
    }
}

void reap_children(int signo) {
    (void)signo;
    while (waitpid(-1, NULL, WNOHANG)>0) {
    }
}

void* thread_main(void* arg){
	int client_fd = *(int*)arg;
	free(arg);

	handle_client(client_fd);
	return NULL;
}