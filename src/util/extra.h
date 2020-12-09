#ifndef EXTRA_H
#define EXTRA_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <botan/secmem.h>

#include <QFileDialog>

std::string atos(int asciiVal);

Botan::secure_vector<uint8_t> toVec(std::string str);
Botan::secure_vector<uint8_t> toVec(char *str, int length);

std::string toStr(Botan::secure_vector<uint8_t> vec);

QString tr(std::string s);
QString tr(const char *s);

std::string newLoc();
std::string getDb();
std::string newKeyFile();
std::string getKeyFile();

#endif
