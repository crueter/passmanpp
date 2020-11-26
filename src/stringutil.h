#ifndef STRINGUTIL_H
#define STRINGUTIL_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <botan/secmem.h>

void replaceAll(std::string& str, const std::string& from, const std::string& to);

std::string trim(const std::string& line);
std::string trimNull(const std::string& line);
std::vector<std::string> split(std::string text, char delim);

std::string atos(int asciiVal);

Botan::secure_vector<uint8_t> toVec(std::string str);
std::string toStr(Botan::secure_vector<uint8_t> vec);

#endif
