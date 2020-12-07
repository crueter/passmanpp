#ifndef SQL_H
#define SQL_H
#include <QMessageBox>
#include <QListWidgetItem>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QVector>

#include "../database.h"

extern QSqlDatabase db;

void dbInit();
QString typeConv(QVariant::Type type);
//int exec(std::string cmd, Database tdb = Database(), bool save = true, int (*callback)(void*, int, char**, char**) = nullptr);

void execAll(std::string stmt);
QString getCreate(QString name, QStringList names, QList<QVariant::Type> types, QVariantList values);
QVector<QSqlQuery> selectAll();
std::string saveSt(Database tdb, bool exec = false);

bool exists(QString field, QString value);

std::vector<std::string> getNames(Database tdb);

#endif
