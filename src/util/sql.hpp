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

#include "../database.hpp"

extern QSqlDatabase db;

void dbInit();

void execAll(const QString &stmt);

const QList<QSqlQuery> selectAll();

bool exists(const QString &field, const QString &value);

#endif
