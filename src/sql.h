#ifndef SQL_H
#define SQL_H
#include <QMessageBox>
#include <QListWidgetItem>
#include <sqlite3.h>

#include "stringutil.h"
#include "database.h"

extern std::ofstream pdb;
extern std::string glob_stList;

extern sqlite3* db;

int _getNames(void *, int, char **data, char **);
int _saveSt(void *, int count, char **data, char **cols);

int exec(std::string cmd, Database tdb = Database(), bool save = true, int (*callback)(void*, int, char**, char**) = nullptr);
void saveSt(Database tdb);
std::vector<std::string> getNames(Database tdb);

#endif
