#include <fstream>
#include <iostream>
#include <sqlite3.h>

#include "stringutil.h"

extern std::ofstream pdb;
extern std::string stList;

extern sqlite3* db;
extern bool modified;

int showNames(void *list, int count, char **data, char **columns);
int showData(void *list, int count, char **data, char **cols);
int _saveSt(void *list, int count, char **data, char **cols);

void saveSt();
int exec(std::string cmd, bool save = true, int (*callback)(void*, int, char**, char**) = nullptr);
