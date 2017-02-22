//
// Created by Nikita on 14.12.15.
//

#include <string.h>
#include "ServerConnection.h"

ServerConnection::ServerConnection(int serverSocket) {

    state = NEW_SERVER_CONNECTION;

    this->serverSocket = serverSocket;

    memset(buffer, 0, sizeof(buffer));

    page = NULL;

}

int ServerConnection::getState() const {

    return state;

}


int ServerConnection::getServerSocket() const {

    return serverSocket;

}

char* ServerConnection::getBuf() {

    return buffer;

}

int ServerConnection::getByteInBuf() {

    return byteInBuf;

}

int ServerConnection::setByteInBuf(int byteInBuf) {

    ServerConnection::byteInBuf = byteInBuf;
    return byteInBuf;

}

void ServerConnection::setState(int state) {

    ServerConnection::state = state;

}

void ServerConnection::setPageStorage(PageStorage* pageStorage) {

    ServerConnection::page = pageStorage;

}

PageStorage* ServerConnection::getPageStorage() {

    return page;

}
