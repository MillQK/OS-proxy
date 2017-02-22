//
// Created by Nikita on 14.12.15.
//

#include "PageStorage.h"

PageStorage::PageStorage(){

    full = false;

    pthread_mutex_init(&pageMutex, NULL);
    pthread_cond_init(&pageCond, NULL);

}

void PageStorage::addItem(char * buf, int size) {

    pthread_mutex_lock(&pageMutex);

    pagePieces.push_back(std::make_pair(buf, size));

    pthread_cond_signal(&pageCond);

    pthread_mutex_unlock(&pageMutex);

}

unsigned long PageStorage::size() {

    pthread_mutex_lock(&pageMutex);

    unsigned long tmp = pagePieces.size();

    pthread_mutex_unlock(&pageMutex);

    return tmp;


}

std::pair<char *, int> PageStorage::getItem(unsigned long pos) { //sdelat thread safe

    while(pos >= pagePieces.size()){

        pthread_cond_wait(&pageCond, &pageMutex);

    }

    return pagePieces.at(pos);

}

bool PageStorage::isFull() {

    pthread_mutex_lock(&pageMutex);

    bool tmp = full;

    pthread_mutex_unlock(&pageMutex);

    return tmp;

}

void PageStorage::setFull(bool f) {

    pthread_mutex_lock(&pageMutex);

    full = f;

    pthread_mutex_unlock(&pageMutex);
}

void PageStorage::lockPageMutex() {

    pthread_mutex_lock(&pageMutex);

}

void PageStorage::unlockPageMutex() {

    pthread_mutex_unlock(&pageMutex);

}
