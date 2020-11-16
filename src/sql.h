#ifndef SQL_H
#define SQL_H
#include <fstream>
#include <iostream>
#include <vector>
#include <sqlite3.h>

#include "stringutil.h"

extern std::ofstream pdb;
extern std::string stList;

extern sqlite3* db;
extern bool modified;

int _getNames(void *list, int count, char **data, char **columns);
int _saveSt(void *list, int count, char **data, char **cols);

int exec(std::string cmd, bool save = true, int (*callback)(void*, int, char**, char**) = nullptr);
void saveSt();
std::vector<std::string> getNames();

#endif
