#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

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
        perrror("bind error");
        exit(1);
    }

    if (listen(server_fd, 1) <0) {
        perror("Listening failed");
        exit(1);
    }

    printf("Listening on port %d...\n", port);

    int client_fd = accept(server_fd, NULL, NULL);
    if (client_fd <0) {
        perror("Accept failed");
        exit(1);
    }

    printf("Client connected\n");

    char buffer[1024];
    read(client_fd, buffer, sizeof(buffer));

    close(client_fd);
    close(server_fd);
}