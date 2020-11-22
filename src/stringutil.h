#ifndef STRINGUTIL_H
#define STRINGUTIL_H
#include <string>
#include <vector>
#include <sstream>

void replaceAll(std::string& str, const std::string& from, const std::string& to);

std::string trim(const std::string& line);
std::vector<std::string> split(std::string text, char delim);

std::string atos(int asciiVal);
unsigned int toShort(char *a);
std::string shortToStr(signed short num);

#endif
