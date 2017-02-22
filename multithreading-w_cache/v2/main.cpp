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

#include <list>
#include <map>
#include <cstring>

#include "ClientConnection.h"

pthread_mutex_t cacheMutex;

struct cmp_str{

    bool operator() (char const *a, char const *b) const{

        return std::strcmp(a,b) < 0;

    }

};

std::map<char*, PageStorage*, cmp_str> cache;

const char* HTTP_DEFAULT_PORT = "80";
const char* HTTP_ERROR_400 = "HTTP/1.0 400";
const char* HTTP_ERROR_405 = "HTTP/1.0 405";
const char* HTTP_ERROR_505 = "HTTP/1.0 505";

ClientConnection* addConnection(int clientSocket) {

    ClientConnection* c = new ClientConnection(clientSocket);

    printf("new connection\n");

    return c;
}

bool isRightUri(ClientConnection* c) {

    char *tmp1 = strchr(c->getBuf(), ' ');

    if (tmp1 == NULL) {

        printf("Bad uri(1)\n");

        return false;
    }

    char *tmp2 = strchr(tmp1 + 1, ' ');

    if (tmp2 == NULL) {

        printf("Bad uri(2)\n");

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

bool isRightRequest(ClientConnection* c) {

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

bool connectWithServer(ClientConnection* c, char* host) {

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

    //serverConnections.push_back(new ServerConnection(serverSocket, c));

    c->setServerConnection(new ServerConnection(serverSocket));

    if (-1 == (connect(serverSocket, (struct sockaddr *) serverAddress, sizeof(*serverAddress)))) {

        perror("Connent to server error\n");

        return false;

    }

    return true;
}


void handleRequest(ClientConnection* c) {

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
            c->setServerConnection(new ServerConnection(-1));
            c->getServerConnection()->setPageStorage(cache.find(c->getUri())->second);

        }

        pthread_mutex_unlock(&cacheMutex);

        free(host);
    }

}

void handleAnswer(ClientConnection* c) {

    if (strstr(c->getServerConnection()->getBuf(), "200") != NULL) {

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

void saveDataToCache(ServerConnection* c) {

    char* tmp = (char*)malloc(c->getByteInBuf() * sizeof(char));

    memcpy(tmp, c->getBuf(), (size_t) c->getByteInBuf());

    c->getPageStorage()->addItem(tmp, c->getByteInBuf());
}

void copyDataToClientBuf(ClientConnection* c) {

    memcpy(c->getBuf(), c->getServerConnection()->getBuf(), (size_t) c->getServerConnection()->getByteInBuf());

    c->setByteInBuf(c->getServerConnection()->getByteInBuf());

    memset(c->getServerConnection()->getBuf(), 0, BUFFERSIZE);

    c->getServerConnection()->setByteInBuf(0);

}

void sendDataToClient(ClientConnection *c) {

    if((c->getState() == ClientConnection::FROM_SERVER) && (c->getByteInBuf() > 0)){

        int result = (int) write(c->getClientSocket(), c->getBuf(), (size_t) c->getByteInBuf());

        if (result == -1) {

            c->setState(ClientConnection::CLIENT_ERROR);

        } else {

            c->setByteInBuf(0);

        }

    }

}

void * thr_func(void* arg){

    ClientConnection * connection = (ClientConnection*) arg;

    int readCount = (int) read(connection->getClientSocket(), connection->getBuf(), BUFFERSIZE);

    if(0 >= readCount){

        printf("Thread read from client error.\n");

        connection->closeConnections();

        delete(connection);

         pthread_exit(NULL);
    }



    connection->setByteInBuf(readCount);

    handleRequest(connection);

    if(ClientConnection::CLIENT_ERROR == connection->getState()){

        printf("Handle request error.\n");

        connection->closeConnections();

        delete(connection);

        pthread_exit(NULL);
    }

    if(ClientConnection::FROM_CACHE == connection->getState()){

        printf("FROOOOMM CACHEEE: %s\n", connection->getUri());

        while (connection->getCurrentCachePosition() < connection->getServerConnection()->getPageStorage()->size() || !connection->getServerConnection()->getPageStorage()->isFull()) {

            connection->getServerConnection()->getPageStorage()->lockPageMutex();

            std::pair<char*, int> pair = connection->getServerConnection()->getPageStorage()->getItem(connection->getCurrentCachePosition());

            if(pair.first == NULL && pair.second == -1){

                connection->getServerConnection()->getPageStorage()->unlockPageMutex();

                connection->closeConnections();

                delete(connection);

                pthread_exit(NULL);

            }

            int result = (int) write(connection->getClientSocket(), pair.first, (size_t) pair.second);

            connection->getServerConnection()->getPageStorage()->unlockPageMutex();

            if (result == -1) {

                printf("Write to client error.\n");

                connection->closeConnections();

                delete(connection);

                pthread_exit(NULL);

            } else {

                connection->incrementCachePosition();

                if (connection->getCurrentCachePosition() == connection->getServerConnection()->getPageStorage()->size() && connection->getServerConnection()->getPageStorage()->isFull()) {

                    printf("Cashe position error ORR END.\n"); //vse otpravili

                    connection->closeConnections();

                    delete(connection);

                    pthread_exit(NULL);

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

            connection->closeConnections();

            delete(connection);

            pthread_exit(NULL);

        } else {

            printf(">>ClientToServer: %s\n", connection->getBuf());

            connection->setByteInBuf(0);

            memset(connection->getBuf(), 0, BUFFERSIZE);

            connection->getServerConnection()->setState(ServerConnection::EXPECTED_RESPONSE);

        }

        int count;
        if (0 >= (count = (int) read(connection->getServerConnection()->getServerSocket(), connection->getServerConnection()->getBuf(), BUFFERSIZE))) {

            connection->setState(ClientConnection::CLIENT_ERROR);
            connection->getServerConnection()->setState(ServerConnection::SERVER_ERROR);

            printf("Response server error.\n");

            connection->closeConnections();

            delete(connection);

            pthread_exit(NULL);

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

            connection->closeConnections();

            pthread_exit(NULL);

        }

        for(;;){

            count = (int) read(connection->getServerConnection()->getServerSocket(), connection->getServerConnection()->getBuf(), BUFFERSIZE);

            if(-1 == count){

                printf("Server read error.\n");

                connection->closeConnections();

                pthread_mutex_lock(&cacheMutex);

                cache.erase(connection->getUri());

                pthread_mutex_unlock(&cacheMutex);

                connection->getServerConnection()->getPageStorage()->deletePage();

                delete(connection);

                pthread_exit(NULL);

            }

            if(0 == count){

                printf("Server read end. URI: %s\n",connection->getUri());

                if(connection->getServerConnection()->getState() == ServerConnection::CACHING_MODE){

                    connection->getServerConnection()->getPageStorage()->setFull(true);

                }

                printf("\nCache size: %ld\n",cache.size());

                for(std::map<char*, PageStorage*>::iterator it = cache.begin(); it != cache.end(); ++it){

                    printf("%s\n",it->first);

                }

                connection->closeConnections();

                pthread_exit(NULL);
            }

            connection->getServerConnection()->setByteInBuf(count);

            if (connection->getServerConnection()->getState() == ServerConnection::CACHING_MODE) {

                saveDataToCache(connection->getServerConnection());
                copyDataToClientBuf(connection);

            } else if (connection->getServerConnection()->getState() == ServerConnection::NOT_CACHING_MODE) {

                copyDataToClientBuf(connection);

            }

            sendDataToClient(connection);


            if(connection->getState() == ClientConnection::CLIENT_ERROR){

                printf("Client write error.\n");

                connection->closeConnections();

                pthread_mutex_lock(&cacheMutex);

                cache.erase(connection->getUri());

                pthread_mutex_unlock(&cacheMutex);

                connection->getServerConnection()->getPageStorage()->deletePage();

                delete(connection);

                pthread_exit(NULL);

            }

        }



    }


}


int main(int argc, char **argv) {

    const char* USAGE = "Usage: Proxy <listenPort>";
    int n = 0;

    if (argc != 2) {

        perror(USAGE);

        exit(EXIT_FAILURE);

    }

    //ProxyFunc *proxyFunc = new ProxyFunc(argv[1]);
    //proxyFunc->work();

    signal(SIGPIPE, SIG_IGN);

    char *listenPortChar = argv[1];

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

    int listenSocket = socket(listenInfo->ai_family, listenInfo->ai_socktype, listenInfo->ai_protocol);

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
        struct sockaddr_in clientAddress;

        socklen_t addrlen = sizeof(clientAddress);

        int newClientFD = accept(listenSocket, (struct sockaddr *) &clientAddress, &addrlen);

        if (newClientFD < 0) {

            perror("Accept error.\n");

            exit(EXIT_FAILURE);
        }

        printf("NEW [AAA CCC CCC EEE PPP TTT]\n");

        ClientConnection* c;
        pthread_t *thread = (pthread_t*)malloc(sizeof(pthread_t));

        if (NULL == (c = addConnection(newClientFD))) {

            perror("Add connection error.\n");

            exit(EXIT_FAILURE);
        }

        int code = pthread_create(thread, NULL, thr_func, (void*)c);
        n++;
        printf("\nThread num: %d\n",n);

        if(0 != code){

            perror("Pthread create error.\n");

            exit(EXIT_FAILURE);
        }

    }

    return EXIT_SUCCESS;
}