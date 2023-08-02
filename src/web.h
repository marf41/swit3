#ifndef SWIT3_WEB_H
#define SWIT3_WEB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>

#include "types.h"

#define MAX_SERVERS 5
#define MAX_CLIENTS 10
#define BUFFER_SIZE 1024

int8_t web_setup(uint8_t sn, uint16_t port);
void web_header(char* resp, uint16_t maxlen, char* s, uint16_t len);
int16_t web_hello(struct Interpreter* ci, struct Request req, char* resp);
int8_t web_loop(struct Interpreter* ci, uint8_t sn, RequestParser* parse);

#endif