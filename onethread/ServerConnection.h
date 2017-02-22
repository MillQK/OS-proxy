//
// Created by Nikita on 13.12.15.
//

#ifndef INC_29_2_SERVERCONNECTION_H
#define INC_29_2_SERVERCONNECTION_H

#include "ClientConnection.h"
#include "PageStorage.h"

#define BUFSIZE 4096

class ServerConnection {

private:

    char buffer[BUFFERSIZE];
    char * uri;
    PageStorage * page;

    int byteInBuf;
    unsigned long currentCachePosition;

    int serverSocket;

    int state;

    ClientConnection* clientConnection;


public:

    const static int NEW_SERVER_CONNECTION = 1;
    const static int EXPECTED_RESPONSE = 2;
    const static int CACHING_MODE = 3;
    const static int NOT_CACHING_MODE = 4;
    const static int SERVER_ERROR = 5;

    ServerConnection(int serverSocket, ClientConnection* connection);

    char* getBuf();

    int getByteInBuf();
    int setByteInBuf(int count);

    int getServerSocket() const;
    ClientConnection *getClientConnection();

    int getState() const;
    void setState(int inState);

    PageStorage* getPageStorage() const;

    void setPageStorage(PageStorage* pageStorage);

};


#endif //INC_29_2_SERVERCONNECTION_H
