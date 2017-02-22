//
// Created by Nikita on 14.12.15.
//

#ifndef INC_30_PAGESTORAGE_H
#define INC_30_PAGESTORAGE_H

#include <vector>
#include <utility>
#include <pthread.h>

class PageStorage {

private:

    bool full, bad;
    std::vector<std::pair<char *, int> > pagePieces;

    pthread_mutex_t pageMutex;
    pthread_cond_t pageCond;


public:

    PageStorage();

    void setFull(bool f);

    bool isFull();

    void addItem(char * buf, int size);

    std::pair<char *, int> getItem(unsigned long pos);

    unsigned long size();

    void lockPageMutex();
    void unlockPageMutex();

    void deletePage();

};

#endif //INC_30_PAGESTORAGE_H
