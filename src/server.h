#ifndef SERVER_H
#define SERVER_H

void server_start(int port);
void handle_client(int client_fd);

#endif