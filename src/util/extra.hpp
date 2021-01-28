#ifndef EXTRA_H
#define EXTRA_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <botan/secmem.h>

#include <QFileDialog>

typedef Botan::secure_vector<uint8_t> secvec;

secvec toVec(const std::string &str);
secvec toVec(const QString &str);
secvec toVec(const char *str, const int length);

const QString tr(const QString &s);
const QString tr(const std::string &s);
const QString tr(const char *s);

const QString newKeyFile();
const QString getKeyFile();
void genKey(const QString &path);

template <typename NumberType = int>
QList<NumberType> range(int start, int amount) {
    QList<NumberType> rangeList;
    for (int i = 0; i < amount; ++i) {
        rangeList.emplaceBack(static_cast<NumberType>(start + i));
    }

    return rangeList;
}

const QString newLoc();
const QString getDb();

void displayErr(const QString &msg);

#endif
