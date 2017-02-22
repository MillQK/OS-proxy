//
// Created by Nikita on 14.12.15.
//

#ifndef INC_30_SERVERCONNECTION_H
#define INC_30_SERVERCONNECTION_H

#include "PageStorage.h"

#define BUFFERSIZE 4096

class ServerConnection {

private:

    char buffer[BUFFERSIZE];
    char * uri;
    PageStorage * page;

    int byteInBuf;
    unsigned long currentCachePosition;

    int serverSocket;

    int state;


public:

    const static int NEW_SERVER_CONNECTION = 1;
    const static int EXPECTED_RESPONSE = 2;
    const static int CACHING_MODE = 3;
    const static int NOT_CACHING_MODE = 4;
    const static int SERVER_ERROR = 5;

    ServerConnection(int serverSocket);

    char* getBuf();

    int getByteInBuf();
    int setByteInBuf(int count);

    int getServerSocket() const;

    int getState() const;
    void setState(int inState);

    PageStorage* getPageStorage();

    void setPageStorage(PageStorage* pageStorage);

};

#endif //INC_30_SERVERCONNECTION_H
