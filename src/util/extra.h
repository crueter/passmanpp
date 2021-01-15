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

secvec toVec(std::string str);
secvec toVec(QString str);
secvec toVec(char *str, int length);

QString tr(QString s);
QString tr(std::string s);
QString tr(const char *s);

QString newKeyFile();
QString getKeyFile();
void genKey(const QString &path);

QString newLoc();
QString getDb();

void displayErr(QString msg);

#endif
