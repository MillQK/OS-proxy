//
// Created by Nikita on 13.12.15.
//

#ifndef INC_29_2_PAGESTORAGE_H
#define INC_29_2_PAGESTORAGE_H

#include <vector>
#include <utility>

class PageStorage {

private:

    bool full;
    std::vector<std::pair<char *, int> > pagePieces;

public:

    PageStorage();

    void setFull(bool full);

    bool isFull() const;

    void addItem(char * buf, int size);

    std::pair<char *, int> getItem(unsigned long pos);

    unsigned long size();

    void deleteAll();

};

#endif //INC_29_2_PAGESTORAGE_H
