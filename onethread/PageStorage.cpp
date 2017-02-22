//
// Created by Nikita on 13.12.15.
//

#include <stdlib.h>
#include "PageStorage.h"

PageStorage::PageStorage(){

    full = false;

}

void PageStorage::addItem(char * buf, int size) {

    pagePieces.push_back(std::make_pair(buf, size));

}

unsigned long PageStorage::size() {

    return pagePieces.size();

}

std::pair<char *, int> PageStorage::getItem(unsigned long pos) {

    return pagePieces.at(pos);

}

bool PageStorage::isFull() const {

    return full;

}

void PageStorage::setFull(bool full) {

    PageStorage::full = full;
}

void PageStorage::deleteAll(){

    for(unsigned long i=0; i<pagePieces.size(); i++){

        free(pagePieces.at(i).first);

    }

    pagePieces.clear();

}