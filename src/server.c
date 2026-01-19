#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <errno.h>

#define MAX_EVENTS 64
#define MAX_CLIENTS 10000
#define BUF_SIZE 4096

void make_nonblocking(int fd);

struct epoll_event events[MAX_EVENTS];

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

	make_nonblocking(server_fd);

	int epoll_fd = epoll_create1(0);
	if (epoll_fd <0) {
		perror("epoll_create failed");
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

    if (listen(server_fd, SOMAXCONN) <0) {
        perror("Listening failed");
        exit(1);
    }

    printf("Listening on port %d...\n", port);

	struct epoll_event ev;
	ev.events = EPOLLIN;
	ev.data.fd = server_fd;
	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &ev);

	while (1) {

		int n = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);

		for (int i = 0; i <n; i++){

			int fd = events[i].data.fd;

	        if (fd == server_fd) {
	        	while (1) {
	        		int client_fd = accept(server_fd, NULL, NULL);
	        		if (client_fd <0) {
	        			if (errno == EAGAIN || errno == EWOULDBLOCK)
	        				break;
	        			perror("accept error");
	        			break;
	        		}

	        		make_nonblocking(client_fd);

	        		if (client_fd >= MAX_CLIENTS) {
	        			close(client_fd);
	        			continue;
	        		}
	        		clients[client_fd].fd = client_fd;
	        		clients[client_fd].buffer_len = 0;

	        		ev.events = EPOLLIN;
	        		ev.data.fd = client_fd;
	        		if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &ev) < 0) {
	        			perror("epoll_ctl failed");
	        			close(client_fd);
	        			continue;
	        		}
	        	}
	        } else {
	            client_t* c = &clients[fd];

	        	if (c->buffer_len == BUF_SIZE) {
	        		epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	        		close(fd);
	        		clients[fd].fd = -1;
	        		clients[fd].buffer_len = 0;
	        		continue;
	        	}

	        	ssize_t bytes_read = read(
	        		fd,
	        		c->buffer + c->buffer_len,
	        		BUF_SIZE - c->buffer_len
	        		);

	            if(bytes_read <=0) {
	            	if (errno == EAGAIN || errno == EWOULDBLOCK)
	            		continue;
	            	epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd, NULL);
	            	close(fd);
	            	clients[fd].fd = -1;
	            	clients[fd].buffer_len = 0;
	            	continue;
	            }

	        	c->buffer_len += bytes_read;

	        	size_t start = 0;

	        	for (size_t i=0; i < c-> buffer_len; i++){
	        		if (c->buffer[i] == '\n') {
	        			size_t msg_len = i - start + 1;

	        			ssize_t n = write(fd, c->buffer + start, msg_len);
	        			if (n <= 0 && errno != EAGAIN) {
	        				close(fd);
	        				clients[fd].fd = -1;
	        			}

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

void make_nonblocking(int fd) {
	int flags = fcntl(fd,F_GETFL, 0);
	fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}
