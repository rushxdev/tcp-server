#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

#define MAX_CLIENTS FD_SETSIZE
#define BUF_SIZE 4096

fd_set master_set;
fd_set read_set;
int max_fd;

typedef struct {
	int fd;
	char buffer[BUF_SIZE];
	size_t buffer_len;
} client_t;

client_t clients[MAX_CLIENTS];

//main server function
void server_start(int port) {
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd <0) {
        perror("Socket creation failed");
        exit(1);
    }

	for (int i = 0; i < MAX_CLIENTS; i++) {
		clients[i].fd = -1;
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

    FD_ZERO(&master_set);
    FD_SET(server_fd, &master_set);
	max_fd = server_fd;

	while (1) {
	    read_set = master_set;

	    int ready = select(max_fd + 1, &read_set, NULL, NULL, NULL);
	    if(ready<0) {
	        perror("Select failed");
	        break;
	    }

	    for(int fd = 0; fd <= max_fd; fd++) {

	        if (!FD_ISSET(fd, &read_set))
	            continue;

	        if (fd == server_fd) {
	            int client_fd = accept(server_fd, NULL, NULL);
	            if (client_fd <0 ) {
	                perror("Accept failed");
	                continue;
	            }

	        	clients[client_fd].fd = client_fd;
	        	clients[client_fd].buffer_len = 0;

	            FD_SET (client_fd, &master_set);
	            if (client_fd > max_fd) {
	                max_fd = client_fd;
	            }

	        } else {
	            client_t* c = &clients[fd];

	        	ssize_t bytes_read = read(fd,
	        		c->buffer + c->buffer_len,
	        		BUF_SIZE - c->buffer_len
	        		);

	            if(bytes_read <=0) {
	                close(fd);
	                FD_CLR(fd, &master_set);
	            	c->fd = -1;
	            }
	        	c->buffer_len += bytes_read;

	        	size_t start = 0;

	        	for (size_t i=0; i < c-> buffer_len; i++){
	        		if (c->buffer[i] == '\n') {
	        			size_t msg_len = i - start + 1;

	        			write(fd, c->buffer + start, msg_len);

	        			start = i +1;
	        		}
	            }

	        	if (start > 0) {
	        		memmove(c-> buffer, c->buffer + start, c->buffer_len - start);
	        		c->buffer_len -= start;
	        	}

	        }
	    }

	}
}
