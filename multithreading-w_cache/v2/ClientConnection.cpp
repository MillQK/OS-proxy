//
// Created by Nikita on 13.12.15.
//

#include <unistd.h>
#include <string.h>
#include "ClientConnection.h"

ClientConnection::ClientConnection(int clientSocket) {

    this->clientSocket = clientSocket;

    memset(buffer, 0, sizeof(buffer));
    byteInBuf = 0;

    uri = NULL;
    serverConnection = NULL;

    currentCachePosition = 0;

    state = NEW_CONNECTION;
}

char* ClientConnection::getBuf() {

    return buffer;

}

int ClientConnection::getByteInBuf() {

    return byteInBuf;

}

int ClientConnection::setByteInBuf(int count) {

    byteInBuf = count;
    return count;

}

int ClientConnection::getClientSocket() const {

    return clientSocket;

}

int ClientConnection::getState() const {

    return state;

}

void ClientConnection::setState(int inState) {

    ClientConnection::state = inState;

}

char* ClientConnection::getUri() const {

    return uri;

}

void ClientConnection::setUri(char* inUri) {

    ClientConnection::uri = inUri;

}

unsigned long ClientConnection::getCurrentCachePosition() const {

    return currentCachePosition;

}

void ClientConnection::incrementCachePosition() {

    currentCachePosition++;

}

ServerConnection* ClientConnection::getServerConnection() {

    return serverConnection;

}

void ClientConnection::setServerConnection(ServerConnection *servConnection) {

    serverConnection=servConnection;

}

void ClientConnection::closeClientConnection() {

    close(clientSocket);

}

void ClientConnection::closeServerConnection() {

    if(serverConnection != NULL){

        if(serverConnection->getServerSocket() > 0)
            close(serverConnection->getServerSocket());

        delete(serverConnection);

    }

}

void ClientConnection::closeConnections() {

    closeServerConnection();
    closeClientConnection();

}
