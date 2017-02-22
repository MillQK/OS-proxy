//
// Created by Nikita on 13.12.15.
//

#ifndef INC_29_2_CLIENTCONNECTION_H
#define INC_29_2_CLIENTCONNECTION_H

#include "PageStorage.h"

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
    PageStorage * page;

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


    PageStorage* getPage() const;

    void setPageStorage(PageStorage* inPage);

    unsigned long getCurrentCachePosition() const;
    void incrementCachePosition();


};


#endif //INC_29_2_CLIENTCONNECTION_H
