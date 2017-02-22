//
// Created by Nikita on 13.12.15.
//

#ifndef INC_29_2_PROXYFUNC_H
#define INC_29_2_PROXYFUNC_H

#include <sys/select.h>
#include <list>
#include <map>
#include <cstring>

#include "ClientConnection.h"
#include "ServerConnection.h"
#include "PageStorage.h"

struct cmp_str{

    bool operator() (char const *a, char const *b) const{

        return std::strcmp(a,b) < 0;

    }

};

class ProxyFunc {

private:

    int maxFd;
    int listenSocket;

    fd_set readfds;
    fd_set writefds;

    std::list<ClientConnection*> clientConnections;
    std::list<ServerConnection*> serverConnections;

    std::map<char*, PageStorage*, cmp_str> cache;

    int checkSocket(int socket);

    void makeFDsForSelect();

    void makeClientFDsForSelect();

    void makeServerFDsForSelect();

    void checkClientsFDs();

    void checkServersFDs();

    ClientConnection* addConnection(int clientSocket);

    void handleRequest(ClientConnection* c);

    bool isRightRequest(ClientConnection* c);

    bool isRightUri(ClientConnection* c) const;

    bool connectWithServer(ClientConnection* c, char* host);

    void handleAnswer(ServerConnection* c);

    void saveDataToCache(ServerConnection* c);

    void copyDataToClientBuf(ServerConnection* c);

public:

    ProxyFunc(char *listenPortChar);

    void work();


};

#endif //INC_29_2_PROXYFUNC_H
