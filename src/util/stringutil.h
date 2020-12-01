#ifndef STRINGUTIL_H
#define STRINGUTIL_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <botan/secmem.h>

void replaceAll(std::string& str, const std::string& from, const std::string& to);

bool find(std::vector<std::string> set, char* cInd);

std::string trim(const std::string& line);
std::string trimNull(const std::string& line);

std::string join(const std::vector<std::string> &elements, const std::string &delim);
std::vector<std::string> split(std::string text, char delim);

std::string atos(int asciiVal);

Botan::secure_vector<uint8_t> toVec(std::string str);
Botan::secure_vector<uint8_t> toVec(char *str, int length);

char *toChar(Botan::secure_vector<uint8_t> vec);
std::string toStr(Botan::secure_vector<uint8_t> vec);

#endif
