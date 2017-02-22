#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include "ProxyFunc.h"


int main(int argc, char **argv) {

    const char* USAGE = "Usage: Proxy <listenPort>";

    if (argc != 2) {

        perror(USAGE);

        exit(EXIT_FAILURE);

    }

    signal(SIGPIPE, SIG_IGN);

    ProxyFunc *proxyFunc = new ProxyFunc(argv[1]);
    proxyFunc->work();

    return EXIT_SUCCESS;
}
