//
// Created by Nikita on 14.12.15.
//

#ifndef INC_30_CLIENTCONNECTION_H
#define INC_30_CLIENTCONNECTION_H

#include "PageStorage.h"
#include "ServerConnection.h"

#define BUFFERSIZE 4096
/*
enum ClientConnectionState {
   NEW_CONNECTION, FROM_CACHE, FROM_SERVER, CLIENT_ERROR
};
*/

class ClientConnection {

private:

    char buffer[BUFFERSIZE];
    char * uri;

    ServerConnection * serverConnection;

    int byteInBuf;
    unsigned long currentCachePosition;

    int clientSocket;

    int state;


public:

    const static int NEW_CONNECTION = 1;
    const static int FROM_CACHE = 2;
    const static int FROM_SERVER = 3;
    const static int CLIENT_ERROR = 4;

    ClientConnection(int clientSocket);

    char* getBuf();

    int getByteInBuf();
    int setByteInBuf(int count);

    int getClientSocket() const;

    int getState() const;
    void setState(int inState);

    char* getUri() const;
    void setUri(char* inUri);

    unsigned long getCurrentCachePosition() const;
    void incrementCachePosition();

    ServerConnection* getServerConnection();
    void setServerConnection(ServerConnection* servConnection);

    void closeClientConnection();
    void closeServerConnection();
    void closeConnections();
};


#endif //INC_30_CLIENTCONNECTION_H
