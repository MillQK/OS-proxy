//
// Created by Nikita on 14.12.15.
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
#include <pthread.h>
#include <semaphore.h>

#include "ProxyFunc.h"

const char* HTTP_DEFAULT_PORT = "80";
const char* HTTP_ERROR_400 = "HTTP/1.0 400";
const char* HTTP_ERROR_405 = "HTTP/1.0 405";
const char* HTTP_ERROR_505 = "HTTP/1.0 505";


/*
ProxyFunc::ProxyFunc(char *listenPortChar){

    int listenPort = atoi(listenPortChar);

    if(listenPort <= 0){

        perror("Wrong port for listening.\n");

        exit(EXIT_FAILURE);

    }

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

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

    pthread_mutex_init(&cacheMutex, NULL);

}*/

//void ProxyFunc::work(){
/*
    int listenPort = atoi(listenPortChar);

    if(listenPort <= 0){

        perror("Wrong port for listening.\n");

        exit(EXIT_FAILURE);

    }

    struct addrinfo hint;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

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

    pthread_mutex_init(&cacheMutex, NULL);

    for (;;) {

/*
        makeFDsForSelect();
        if (-1 == select(maxFd + 1, &readfds, &writefds, NULL, NULL)) {

            perror("Select error.\n");

            exit(EXIT_FAILURE);
        }

        checkServersFDs();
        checkClientsFDs();
*/
/*        struct sockaddr_in clientAddress;

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

        ClientConnection* c;
        pthread_t thread;

        if (NULL == (c = addConnection(newClientFD))) {

            perror("Add connection error.\n");

            exit(EXIT_FAILURE);
        }

        int code = pthread_create(&thread, NULL, ProxyFunc::thr_fun, (void*)c);

        if(0 != code){

            perror("Pthread create error.\n");

            exit(EXIT_FAILURE);
        }

    }
}
*/

/*
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

        printf("kill bad client connection\n");

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
                    if (c->getCurrentCachePosition() < c->getPage()->size()) {

                        std::pair<char*, int> pair = c->getPage()->getItem(c->getCurrentCachePosition());

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

                    if (0 == (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(), BUFSIZE)))) {

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
                    if (0 == (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(), BUFSIZE)))) {

                        c->getClientConnection()->setState(ClientConnection::CLIENT_ERROR);
                        c->setState(ServerConnection::SERVER_ERROR);
                        c->getPageStorage()->setFull(true);

                    } else {

                        saveDataToCache(c);
                        copyDataToClientBuf(c);

                    }
                }

                break;

            case ServerConnection::NOT_CACHING_MODE:

                if (FD_ISSET(c->getServerSocket(), &readfds)) {
                    if (0 == (c->setByteInBuf((int) read(c->getServerSocket(), c->getBuf(), BUFSIZE)))) {

                        c->getClientConnection()->setState(ClientConnection::CLIENT_ERROR);
                        c->setState(ServerConnection::SERVER_ERROR);

                    } else {

                        copyDataToClientBuf(c);

                    }
                }
        }
    }
}
*/
ClientConnection* ProxyFunc::addConnection(int clientSocket) {

    ClientConnection* c = new ClientConnection(clientSocket);
    clientConnections.push_back(c);

    printf("new connection\n");

    return c;
}

void ProxyFunc::handleRequest(ClientConnection* c) {

    printf("%s\n", c->getBuf());

    if (isRightRequest(c)) {

        char* host = (char*)calloc(BUFFERSIZE, sizeof(char));

        sscanf(c->getUri(), "%*[^/]%*[/]%[^/]", host);
        printf("host: %s\n", host);

        pthread_mutex_lock(&cacheMutex);

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

        pthread_mutex_unlock(&cacheMutex);

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

bool ProxyFunc::isRightUri(ClientConnection* c) {

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

    size_t length = (size_t) (endIndex - 1);

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

    //serverConnections.push_back(new ServerConnection(serverSocket, c));

    c->setServerConnection(new ServerConnection(serverSocket));

    if (-1 == (connect(serverSocket, (struct sockaddr *) serverAddress, sizeof(*serverAddress)))) {

        perror("Connent to server error\n");

        return false;

    }

    return true;
}

void ProxyFunc::handleAnswer(ClientConnection* c) {

    if (strstr(c->getBuf(), "200") != NULL) {

        c->getServerConnection()->setState(ServerConnection::CACHING_MODE);

        PageStorage * pageStorage = new PageStorage();

        pthread_mutex_lock(&cacheMutex);

        cache.insert(std::pair<char*, PageStorage*>(c->getUri(), pageStorage));

        pthread_mutex_unlock(&cacheMutex);

        c->getServerConnection()->setPageStorage(pageStorage);

    } else {

        c->getServerConnection()->setState(ServerConnection::NOT_CACHING_MODE);

    }

}

void ProxyFunc::saveDataToCache(ServerConnection* c) {

    char* tmp = (char*)malloc(c->getByteInBuf() * sizeof(char));

    memcpy(tmp, c->getBuf(), (size_t) c->getByteInBuf());

    c->getPageStorage()->addItem(tmp, c->getByteInBuf());
}

void ProxyFunc::copyDataToClientBuf(ClientConnection* c) {

    memcpy(c->getBuf(), c->getServerConnection()->getBuf(), (size_t) c->getServerConnection()->getByteInBuf());

    c->setByteInBuf(c->getServerConnection()->getByteInBuf());

    memset(c->getServerConnection()->getBuf(), 0, BUFFERSIZE);

    c->getServerConnection()->setByteInBuf(0);

}

void ProxyFunc::sendDataToClient(ClientConnection *c) {

    if((c->getState() == ClientConnection::FROM_SERVER) && (c->getByteInBuf() > 0)){

        int result = (int) write(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf());

        if (result == -1) {

            c->setState(ClientConnection::CLIENT_ERROR);

        } else {

            c->setByteInBuf(0);

        }

    }

}

void ProxyFunc::thread_func(void *arg) {

    ClientConnection * connection = (ClientConnection*) arg;

    int readCount = (int) read(connection->getClientSocket(), connection->getBuf(), BUFFERSIZE);

    if(0 == readCount){

        printf("Thread read from client error.\n");

        return;
    }

    connection->setByteInBuf(readCount);

    handleRequest(connection);

    if(ClientConnection::CLIENT_ERROR == connection->getState()){

        printf("Handle request error.\n");

        return;
    }

    if(ClientConnection::FROM_CACHE == connection->getState()){

        while (connection->getCurrentCachePosition() < connection->getPage()->size()) {

            connection->getPage()->lockPageMutex();

            std::pair<char*, int> pair = connection->getPage()->getItem(connection->getCurrentCachePosition());

            int result = (int) write(connection->getClientSocket(), pair.first, (size_t) pair.second);

            connection->getPage()->unlockPageMutex();

            if (result == -1) {

                printf("Write to client error.\n");

                return;

            } else {

                connection->incrementCachePosition();

                if (connection->getCurrentCachePosition() == connection->getPage()->size() && connection->getPage()->isFull()) {

                    printf("Cashe position error.\n"); //vse otpravili

                    return;

                }

            }

        }

    }

    if(ClientConnection::FROM_SERVER == connection->getState()){

        int result = (int) write(connection->getServerConnection()->getServerSocket(), connection->getBuf(), (size_t) connection->getByteInBuf());

        if (result == -1) {

            connection->setState(ClientConnection::CLIENT_ERROR);
            connection->getServerConnection()->setState(ServerConnection::SERVER_ERROR);

            printf("Server write error.\n");

        } else {

            printf(">>ClientToServer: %s\n", connection->getBuf());

            connection->setByteInBuf(0);

            memset(connection->getBuf(), 0, BUFFERSIZE);

            connection->getServerConnection()->setState(ServerConnection::EXPECTED_RESPONSE);

        }

        int count;
        if (0 == (count = (int) read(connection->getServerConnection()->getServerSocket(), connection->getServerConnection()->getBuf(), BUFFERSIZE))) {

            connection->setState(ClientConnection::CLIENT_ERROR);
            connection->getServerConnection()->setState(ServerConnection::SERVER_ERROR);

            printf("Response server error.\n");

            return;

        } else {

            connection->getServerConnection()->setByteInBuf(count);

            handleAnswer(connection);

            if (connection->getServerConnection()->getState() == ServerConnection::CACHING_MODE) {

                saveDataToCache(connection->getServerConnection());
                copyDataToClientBuf(connection);

            } else if (connection->getServerConnection()->getState() == ServerConnection::NOT_CACHING_MODE) {

                copyDataToClientBuf(connection);

            }

            sendDataToClient(connection);


        }

        if(connection->getState()==ClientConnection::CLIENT_ERROR || connection->getServerConnection()->getState()==ServerConnection::SERVER_ERROR){

            printf("Something wrong.\n");

            return;

        }

        for(;;){

            count = (int) read(connection->getServerConnection()->getServerSocket(), connection->getServerConnection()->getBuf(), BUFFERSIZE);

            if(0 == count){

                printf("Server read error\n");

                return;
            }

            connection->getServerConnection()->setByteInBuf(count);

            if (connection->getServerConnection()->getState() == ServerConnection::CACHING_MODE) {

                saveDataToCache(connection->getServerConnection());
                copyDataToClientBuf(connection);

            } else if (connection->getServerConnection()->getState() == ServerConnection::NOT_CACHING_MODE) {

                copyDataToClientBuf(connection);

            }

            sendDataToClient(connection);


        }



    }

}

void ProxyFunc::initCacheMutex() {

    pthread_mutex_init(&cacheMutex, NULL);

}