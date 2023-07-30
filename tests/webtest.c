#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "web.c"

int main() {
    web_setup(1, 8080);
    printf("START\n");
    while(1) {
        web_loop(1, web_hello);
        printf(".");
    }
}