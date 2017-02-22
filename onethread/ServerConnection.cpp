//
// Created by Nikita on 13.12.15.
//

#include "ServerConnection.h"
#include <string.h>

ServerConnection::ServerConnection(int serverSocket, ClientConnection* connection) {

    state = NEW_SERVER_CONNECTION;

    this->serverSocket = serverSocket;

    memset(buffer, 0, sizeof(buffer));
    this->clientConnection = connection;

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


ClientConnection *ServerConnection::getClientConnection() {

    return clientConnection;

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

PageStorage* ServerConnection::getPageStorage() const {

    return page;

}
