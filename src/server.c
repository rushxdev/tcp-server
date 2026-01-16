#include "server.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>

fd_set master_set;
fd_set read_set;
int max_fd;

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

	            FD_SET (client_fd, &master_set);
	            if (client_fd > max_fd) {
	                max_fd = client_fd;
	            }

	        } else {
	            char buffer[1024];
	            ssize_t bytes_read = read(fd, buffer, sizeof(buffer));

	            if(bytes_read <=0) {
	                close(fd);
	                FD_CLR(fd, &master_set);
	            } else {
	                write(fd, buffer, bytes_read);
	            }
	        }
	    }

	}
}
