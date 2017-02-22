//
// Created by Nikita on 13.12.15.
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <iostream>
#include <netdb.h>
#include <string.h>
#include <arpa/inet.h>

#include "ProxyFunc.h"

const char* HTTP_DEFAULT_PORT = "80";
const char* HTTP_ERROR_400 = "HTTP/1.0 400";
const char* HTTP_ERROR_405 = "HTTP/1.0 405";
const char* HTTP_ERROR_505 = "HTTP/1.0 505";

ProxyFunc::ProxyFunc(char *listenPortChar){

    int listenPort = atoi(listenPortChar);

    if(listenPort <= 0){

        perror("Wrong port for listening.\n");

        exit(EXIT_FAILURE);

    }

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;//ipv4
    hint.ai_socktype = SOCK_STREAM;//socket type, Обеспечивает создание двусторонних надежных и последовательных потоков байтов , поддерживающих соединения. Может также поддерживаться механизм внепоточных данных.
    hint.ai_flags = AI_PASSIVE;//сетевой адрес каждой структуры не будет указан

    int status;
    struct addrinfo *listenInfo;

    if ((status = getaddrinfo(NULL, listenPortChar, &hint, &listenInfo)) != 0) {

        perror("Getaddrinfo error.\n");

        exit(EXIT_FAILURE);
    }

    listenSocket = socket(listenInfo->ai_family, listenInfo->ai_socktype, listenInfo->ai_protocol);

    if (-1 == checkSocket(listenSocket)) {

        perror("Check socket error.\n");

        exit(EXIT_FAILURE);
    }

    int tmp = 1;
    if (-1 == setsockopt(listenSocket, SOL_SOCKET, SO_REUSEADDR, &tmp, sizeof(int))) { // change socket flags, sol_socket - flag level socket

        perror("Setsockopt error.\n");                                                      //so_reuseaddr - you can use local address again

        exit(EXIT_FAILURE);
    }

    if (-1 == bind(listenSocket, listenInfo->ai_addr, listenInfo->ai_addrlen)) {

        perror("Bind error.\n");

        exit(EXIT_FAILURE);
    }

    if ((listen(listenSocket, SOMAXCONN)) == -1) {

        perror("Listen error.\n");

        exit(EXIT_FAILURE);
    }

}

void ProxyFunc::work(){
    for (;;) {
        makeFDsForSelect();
        if (-1 == select(maxFd + 1, &readfds, &writefds, NULL, NULL)) {

            perror("Select error.\n");

            exit(EXIT_FAILURE);
        }

        checkServersFDs();
        checkClientsFDs();

        struct sockaddr_in clientAddress;

        if (FD_ISSET(listenSocket, &readfds)) {

            socklen_t addrlen = sizeof(clientAddress);

            int newClientFD = accept(listenSocket, (struct sockaddr *) &clientAddress, &addrlen);

            if (newClientFD < 0) {

                perror("Accept error.\n");

                exit(EXIT_FAILURE);
            }
            if (-1 == checkSocket(newClientFD)) {

                perror("Check socket error.\n");

                exit(EXIT_FAILURE);
            }
            if (NULL == addConnection(newClientFD)) {

                perror("Add connection error.\n");

                exit(EXIT_FAILURE);
            }
        }
    }
}

int ProxyFunc::checkSocket(int socket) {

    if (socket >= FD_SETSIZE) {

        return -1;
    }

    if (socket > maxFd) {

        maxFd = socket;
    }

    return 0;
}

void ProxyFunc::makeFDsForSelect(){

    FD_ZERO(&readfds);
    FD_ZERO(&writefds);

    FD_SET(listenSocket, &readfds);

    makeClientFDsForSelect();
    makeServerFDsForSelect();

}

void ProxyFunc::makeClientFDsForSelect(){

    std::list<ClientConnection*> badConnections;

    for (std::list<ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); ++it) {

        ClientConnection* c = *it;

        switch (c->getState()) {

            case ClientConnection::CLIENT_ERROR:

                badConnections.push_back(c);
                break;

            case ClientConnection::NEW_CONNECTION:

                FD_SET(c->getClientSocket(), &readfds);
                break;

            default:

                if (c->getByteInBuf() > 0) {

                    FD_SET(c->getClientSocket(), &writefds);
                }
                break;
        }

    }

    for (std::list<ClientConnection*>::iterator it = badConnections.begin(); it != badConnections.end(); ++it) {

        ClientConnection* c = *it;
        close(c->getClientSocket());

        clientConnections.remove(c);

        delete (c);

        printf("kill bad client connection.\n");

    }
}

void ProxyFunc::makeServerFDsForSelect(){

    std::list<ServerConnection*> badConnections;

    for (std::list<ServerConnection*>::iterator it = serverConnections.begin(); it != serverConnections.end(); ++it) {

        ServerConnection *c = *it;

        switch (c->getState()) {

            case ServerConnection::SERVER_ERROR:

                badConnections.push_back(c);
                break;

            case ServerConnection::NEW_SERVER_CONNECTION:

                FD_SET(c->getServerSocket(), &writefds);
                break;

            default:

                if (c->getByteInBuf() == 0 && c->getClientConnection()->getByteInBuf() == 0) {

                    FD_SET(c->getServerSocket(), &readfds);
                }
                break;

        }

    }

    for (std::list<ServerConnection*>::iterator it = badConnections.begin(); it != badConnections.end(); ++it) {

        ServerConnection* c = *it;
        close(c->getServerSocket());

        serverConnections.remove(c);

        delete (c);

        printf("kill bad server connection\n");

    }
}

void ProxyFunc::checkClientsFDs(){

    for (std::list<ClientConnection*>::iterator it = clientConnections.begin(); it != clientConnections.end(); ++it) {

        ClientConnection *c = *it;

        int state = c->getState();

        switch (state) {

            case ClientConnection::NEW_CONNECTION:

                if (FD_ISSET(c->getClientSocket(), &readfds)) {
                    if (0 == (c->setByteInBuf((int) read(c->getClientSocket(), c->getBuf(), BUFSIZE)))) {

                        c->setState(ClientConnection::CLIENT_ERROR);

                    } else {

                        handleRequest(c);

                    }
                }
                break;

            case ClientConnection::FROM_CACHE:

                if (FD_ISSET(c->getClientSocket(), &writefds)) {
                    if(c->getPage() != NULL) {
                        if (c->getCurrentCachePosition() < c->getPage()->size()) {

                            std::pair<char *, int> pair = c->getPage()->getItem(c->getCurrentCachePosition());

                            int result = (int) write(c->getClientSocket(), pair.first, (size_t) pair.second);

                            if (result == -1) {

                                c->setState(ClientConnection::CLIENT_ERROR);

                            } else {

                                c->incrementCachePosition();

                                if (c->getCurrentCachePosition() == c->getPage()->size() && c->getPage()->isFull()) {

                                    c->setState(ClientConnection::CLIENT_ERROR);

                                }

                            }

                        }
                    } else {

                        c->setState(ClientConnection::CLIENT_ERROR);

                    }
                }
                break;

            case ClientConnection::FROM_SERVER:

                if (c->getByteInBuf() > 0 && FD_ISSET(c->getClientSocket(), &writefds)) {

                    int result = (int) write(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf());

                    if (result == -1) {

                        c->setState(ClientConnection::CLIENT_ERROR);

                    } else {

                        c->setByteInBuf(0);

                    }
                    //printf("<<ServerTOClient: %s\n", c->getBuf());
                }
                break;

            case ClientConnection::CLIENT_ERROR:
                break;
        }

    }


}

void ProxyFunc::checkServersFDs(){

    for (std::list<ServerConnection*>::iterator it = serverConnections.begin(); it != serverConnections.end(); ++it) {

        ServerConnection* c = *it;

        switch (c->getState()) {

            case ServerConnection::SERVER_ERROR:

                break;

            case ServerConnection::NEW_SERVER_CONNECTION:

                if (FD_ISSET(c->getServerSocket(), &writefds)) {

                    int result = (int) write(c->getServerSocket(), c->getClientConnection()->getBuf(), (size_t) c->getClientConnection()->getByteInBuf());

                    if (result == -1) {

                        c->getClientConnection()->setState(ClientConnection::CLIENT_ERROR);
                        c->setState(ServerConnection::SERVER_ERROR);

                    } else {

                        printf(">>ClientToServer: %s\n", c->getClientConnection()->getBuf());

                        c->getClientConnection()->setByteInBuf(0);

                        memset(c->getClientConnection()->getBuf(), 0, BUFSIZE);

                        c->setState(ServerConnection::EXPECTED_RESPONSE);

                    }
                }

                break;

            case ServerConnection::EXPECTED_RESPONSE:

                if (FD_ISSET(c->getServerSocket(), &readfds)) {

                    if (0 >= (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(), BUFSIZE)))) {

                        c->getClientConnection()->setState(ClientConnection::CLIENT_ERROR);
                        c->setState(ServerConnection::SERVER_ERROR);

                    } else {

                        handleAnswer(c);

                        if (c->getState() == ServerConnection::CACHING_MODE) {

                            saveDataToCache(c);
                            copyDataToClientBuf(c);

                        } else if (c->getState() == ServerConnection::NOT_CACHING_MODE) {

                            copyDataToClientBuf(c);

                        }

                    }
                }

                break;

            case ServerConnection::CACHING_MODE:

                if (FD_ISSET(c->getServerSocket(), &readfds)) {
                    int count = (int) read(c->getServerSocket(), c->getBuf(), BUFSIZE);
                    if (0 == count) {

                        c->getClientConnection()->setState(ClientConnection::CLIENT_ERROR);
                        c->setState(ServerConnection::SERVER_ERROR);
                        c->getPageStorage()->setFull(true);

                    } else {
                        if(-1 == count){

                            cache.erase(c->getClientConnection()->getUri());
                            c->getPageStorage()->deleteAll();
                            delete(c->getPageStorage());
                            c->getClientConnection()->setState(ClientConnection::CLIENT_ERROR);
                            c->setState(ServerConnection::SERVER_ERROR);

                        }else {

                            c->setByteInBuf(count);
                            saveDataToCache(c);
                            copyDataToClientBuf(c);

                        }

                    }
                }

                break;

            case ServerConnection::NOT_CACHING_MODE:

                if (FD_ISSET(c->getServerSocket(), &readfds)) {
                    if (0 >= (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(), BUFSIZE)))) {

                        c->getClientConnection()->setState(ClientConnection::CLIENT_ERROR);
                        c->setState(ServerConnection::SERVER_ERROR);

                    } else {

                        copyDataToClientBuf(c);

                    }
                }
        }
    }
}

ClientConnection* ProxyFunc::addConnection(int clientSocket) {

    ClientConnection* c = new ClientConnection(clientSocket);
    clientConnections.push_back(c);

    printf("new connection\n");

    return c;
}

void ProxyFunc::handleRequest(ClientConnection* c) {

    printf("%s\n", c->getBuf());

    if (isRightRequest(c)) {

        char* host = (char*)calloc(BUFSIZE, sizeof(char));

        sscanf(c->getUri(), "%*[^/]%*[/]%[^/]", host);
        printf("host: %s\n", host);

        if (cache.find(c->getUri()) == cache.end()) { //check cache

            c->setState(ClientConnection::FROM_SERVER);

            if (!connectWithServer(c, host)) {

                printf("Connect with server error.\n");
                c->setState(ClientConnection::CLIENT_ERROR);

            }

        } else {

            c->setState(ClientConnection::FROM_CACHE);
            c->setPageStorage(cache.find(c->getUri())->second);

        }

        free(host);
    }

}

bool ProxyFunc::isRightRequest(ClientConnection* c) {

    if (NULL == strstr(c->getBuf(), "GET") && NULL == strstr(c->getBuf(), "HEAD")) {

        printf("Method must be GET or HEAD\n");

        write(c->getClientSocket(), HTTP_ERROR_405, sizeof(HTTP_ERROR_405));
        c->setState(ClientConnection::CLIENT_ERROR);

        return false;
    }

    if (NULL == strstr(c->getBuf(), "HTTP/1.0")) {

        printf("Protocol must be HTTP/1.0\n");

        write(c->getClientSocket(), HTTP_ERROR_505, sizeof(HTTP_ERROR_505));
        c->setState(ClientConnection::CLIENT_ERROR);

        return false;
    }

    if (!isRightUri(c)) {

        printf("Uri error\n");

        write(c->getClientSocket(), HTTP_ERROR_400, sizeof(HTTP_ERROR_400));
        c->setState(ClientConnection::CLIENT_ERROR);

        return false;
    }

    return true;
}

bool ProxyFunc::isRightUri(ClientConnection* c) const {

    char *tmp1 = strchr(c->getBuf(), ' ');

    if (tmp1 == NULL) {

        printf("Bad url(1)\n");

        return false;
    }

    char *tmp2 = strchr(tmp1 + 1, ' ');

    if (tmp2 == NULL) {

        printf("Bad url(2)\n");

        return false;
    }

    int endIndex = (int) (tmp2 - tmp1);

    size_t length = (size_t)(endIndex - 1);

    char* uri = (char *) calloc(length, sizeof(char));

    strncpy(uri, tmp1 + 1, length);
    c->setUri(uri);
    printf("uri: %s\n", uri);

    return true;
}

bool ProxyFunc::connectWithServer(ClientConnection* c, char* host) {

    struct addrinfo *remoteInfo;

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;

    int status;

    if ((status = getaddrinfo(host, HTTP_DEFAULT_PORT, &hint, &remoteInfo)) != 0) {

        perror("Getaddrinfo error.\n");

        return false;

    }
    struct sockaddr_in *serverAddress = (struct sockaddr_in *) remoteInfo->ai_addr;

    // IP to string
    void* address = &(serverAddress->sin_addr);
    char buffer[40];

    inet_ntop(remoteInfo->ai_family, address, buffer, sizeof(buffer));
    printf("ip: %s\n", buffer);

    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (-1 == checkSocket(serverSocket)) {

        perror("Check socket error\n");

        return false;
    }

    serverConnections.push_back(new ServerConnection(serverSocket, c));

    if (-1 == (connect(serverSocket, (struct sockaddr *) serverAddress, sizeof(*serverAddress)))) {

        perror("Connent to server error\n");

        return false;

    }

    return true;
}

void ProxyFunc::handleAnswer(ServerConnection* c) {

    if (strstr(c->getBuf(), "200") != NULL) {

        c->setState(ServerConnection::CACHING_MODE);

        PageStorage * pageStorage = new PageStorage();

        cache.insert(std::pair<char*, PageStorage*>(c->getClientConnection()->getUri(), pageStorage));

        c->setPageStorage(pageStorage);

    } else {

        c->setState(ServerConnection::NOT_CACHING_MODE);

    }

}

void ProxyFunc::saveDataToCache(ServerConnection* c) {

    char* tmp = (char*)malloc(c->getByteInBuf() * sizeof(char));

    memcpy(tmp, c->getBuf(), (size_t) c->getByteInBuf());

    c->getPageStorage()->addItem(tmp, c->getByteInBuf());
}

void ProxyFunc::copyDataToClientBuf(ServerConnection* c) {

    memcpy(c->getClientConnection()->getBuf(), c->getBuf(), (size_t) c->getByteInBuf());

    c->getClientConnection()->setByteInBuf(c->getByteInBuf());

    memset(c->getBuf(), 0, BUFSIZE);

    c->setByteInBuf(0);

}