#ifndef SQL_H
#define SQL_H
#include <QMessageBox>
#include <QListWidgetItem>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QDebug>

#include "../database.h"

extern QSqlDatabase db;

void dbInit();

void execAll(QString stmt);
void execAll(std::string stmt);
QString getCreate(QString name, QStringList names, QList<QMetaType> types, QVariantList values);
QList<QSqlQuery> selectAll();
std::string saveSt(bool exec = false);

bool exists(QString field, QString value);

#endif
