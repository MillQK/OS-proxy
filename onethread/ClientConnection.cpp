//
// Created by Nikita on 13.12.15.
//

#include "ClientConnection.h"
#include <string.h>

ClientConnection::ClientConnection(int clientSocket) {

    this->clientSocket = clientSocket;

    memset(buffer, 0, sizeof(buffer));
    byteInBuf = 0;

    uri = NULL;
    page = NULL;

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

PageStorage* ClientConnection::getPage() const {

    return page;

}

void ClientConnection::setPageStorage(PageStorage *inPage){

    ClientConnection::page = inPage;

}

unsigned long ClientConnection::getCurrentCachePosition() const {

    return currentCachePosition;

}

void ClientConnection::incrementCachePosition() {

    currentCachePosition++;

}
