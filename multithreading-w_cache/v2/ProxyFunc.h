//
// Created by Nikita on 14.12.15.
//

#ifndef INC_30_PROXYFUNC_H
#define INC_30_PROXYFUNC_H

#include <sys/select.h>
#include <list>
#include <map>
#include <cstring>

#include "ClientConnection.h"
#include "PageStorage.h"

class ProxyFunc {

public:
/*
    static int maxFd;
    static int listenSocket;

    fd_set readfds;
    fd_set writefds;

    static std::list<ClientConnection*> clientConnections;
    static std::list<ServerConnection*> serverConnections;

    static pthread_mutex_t cacheMutex;

    static std::map<char*, PageStorage*> cache;
    */

    static int checkSocket(int socket);

    void makeFDsForSelect();

    void makeClientFDsForSelect();

    void makeServerFDsForSelect();

    void checkClientsFDs();

    void checkServersFDs();

    static ClientConnection* addConnection(int clientSocket);

    static void handleRequest(ClientConnection* c);

    static bool isRightRequest(ClientConnection* c);

    static bool isRightUri(ClientConnection* c);

    static bool connectWithServer(ClientConnection* c, char* host);

    static void handleAnswer(ClientConnection* c);

    static void saveDataToCache(ServerConnection* c);

    static void copyDataToClientBuf(ClientConnection* c);

    static void sendDataToClient(ClientConnection* c);

//    ProxyFunc(char *listenPortChar);

    static void work();

    static void thread_func(void *arg);

    static void initCacheMutex();

    //static int maxFd;
    static int listenSocket;

    fd_set readfds;
    fd_set writefds;

    static std::list<ClientConnection*> clientConnections;
    static std::list<ServerConnection*> serverConnections;

    static pthread_mutex_t cacheMutex;

    static std::map<char*, PageStorage*> cache;


};

#endif //INC_30_PROXYFUNC_H
