#ifndef DB_H
#define DB_H
#include "sql.h"

QChar randomChar();

QString genPass(int length, bool capitals, bool numbers, bool symbols);
void genKey(const QString &path);

QString randomPass();

#endif
