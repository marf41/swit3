#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#define MAX_SERVERS 5
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int server_fd[MAX_SERVERS], new_socket, activity, i, valread, sd;
int client_sockets[MAX_SERVERS][MAX_CLIENTS] = {0};

typedef int16_t (RequestParser)(char* req, uint16_t reqlen, char* resp, uint16_t maxlen);

int8_t web_setup(uint8_t sn, uint16_t port) {
    if (port <= 0) { return -1; }

    if ((server_fd[sn] = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Socket creation failed");
        return -2;
    }

    int opt = 1;
    if (setsockopt(server_fd[sn], SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("Setsockopt failed");
        return -3;
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_fd[sn], (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        return -4;
    }

    if (fcntl(server_fd[sn], F_SETFL, O_NONBLOCK) < 0) {
        perror("Failed to set server socket as nonblocking");
        return -5;
    }

    if (listen(server_fd[sn], 3) < 0) {
        perror("Listen failed");
        return -6;
    }

    return 0;
}

void web_header(char* resp, uint16_t maxlen, char* s, uint16_t len) {
    snprintf(resp, maxlen, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: %d\n\n%s", len, s);
}

int16_t web_hello(char* req, uint16_t reqlen, char* resp, uint16_t maxlen) {
    // snprintf(resp, maxlen, "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello, World!");
    web_header(resp, maxlen, "Hello, World!", 13);
    return -strlen(resp);
}

int8_t web_loop(uint8_t sn, RequestParser* parse) {
    int8_t reqs = 0;
    fd_set readfds;
    char buffer[BUFFER_SIZE] = { 0 };
    char response[BUFFER_SIZE] = { 0 };
    
    if ((new_socket = accept(server_fd[sn], NULL, NULL)) >= 0) {
        if (fcntl(new_socket, F_SETFL, O_NONBLOCK) < 0) {
            perror("Failed to set client socket as nonblocking");
            return -1;
        }
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[sn][i] == 0) { client_sockets[sn][i] = new_socket; break; }
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        sd = client_sockets[sn][i];

        if (sd > 0) {
            valread = read(sd, buffer, BUFFER_SIZE);
            if (valread > 0) {
                int16_t len = parse(buffer, valread, response, BUFFER_SIZE);
                // if (len < 0) { printf("Response %d: %s\n", len, response); }
                if (len == 0) { return len; }
                reqs++;
                uint16_t sent = 0;
                uint16_t alen = (len < 0) ? (-len) : len;
                while (sent < alen) {
                    uint16_t s = send(sd, response + sent, alen - sent, 0);
                    if (s < 0) { close(sd); perror("Send error"); break; }
                    sent += s;
                }
                if (len < 0) { close(sd); client_sockets[sn][i] = 0; }
            } else if (valread == 0) {
                close(sd);
                client_sockets[sn][i] = 0;
            } else {
                if (errno != EAGAIN && errno != EWOULDBLOCK) {
                    perror("Read error");
                    close(sd);
                    client_sockets[sn][i] = 0;
                }
            }
        }
    }
/*
    FD_ZERO(&readfds);
    FD_SET(server_fd, &readfds);
    int max_sd = server_fd;

    for (i = 0; i < MAX_CLIENTS; i++) {
        sd = client_sockets[i];
        if (sd > 0) { FD_SET(sd, &readfds); }
        if (sd > max_sd) { max_sd = sd; }
    }

    activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR)) { perror("Select error"); }
    if (FD_ISSET(server_fd, &readfds)) {
        struct sockaddr_in client_addr;
        socklen_t client_addrlen = sizeof(client_addr);
        if ((new_socket = accept(server_fd, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) {
            perror("Accept failed");
            return -1;
        }
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (client_sockets[i] == 0) { client_sockets[i] = new_socket; break; }
        }
    }

    for (i = 0; i < MAX_CLIENTS; i++) {
        sd = client_sockets[i];
        if (FD_ISSET(sd, &readfds)) {
            valread = read(sd, buffer, BUFFER_SIZE);
            printf("Got %d: %s\n", i, buffer);
            reqs++;
            send(sd, response, strlen(response), 0);
            close(sd);
            client_sockets[i] = 0;
        }
    }
*/

    return reqs;
}