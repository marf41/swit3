#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#include "types.h"
#include "web.h"

int main() {
    web_setup(1, 8080);
    printf("START\n");
    struct Interpreter ci;
    while(1) {
        web_loop(&ci, 1, web_hello);
        printf(".");
    }
}