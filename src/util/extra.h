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

QString atos(int asciiVal);

secvec toVec(std::string str);
secvec toVec(QString str);
secvec toVec(char *str, int length);

QByteArray toQBA(secvec vec);
QString toStr(secvec vec);
std::string toStdStr(secvec vec);

QString tr(QString s);
QString tr(const char *s);

QString newLoc();
QString getDb();
QString newKeyFile();
QString getKeyFile();

void displayErr(QString msg);

#endif
