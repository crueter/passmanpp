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

void execAll(const QString &stmt);

QList<QSqlQuery> selectAll();

bool exists(const QString &field, const QString &value);

#endif
