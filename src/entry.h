#ifndef ENTRY_H
#define ENTRY_H
#include <QTableWidgetItem>

#include "sql.h"
#include "field.h"
#include "database.h"

void redrawTable(QTableWidget *table, Database *tdb);

class Entry : public QObject
{
    QList<Field *> fields;
    Database *database;
    QString name;
public:
    Entry(QList<Field *> fields, Database *tdb);

    void addField(Field *field);
    bool removeField(Field *field);
    int indexOf(Field *field);
    Field *fieldNamed(QString name);
    Field *fieldAt(int index);

    QList<Field *> &getFields();
    QList<Field *> &setFields(QList<Field *> &fields);
    int fieldLength();

    Database *getDb();
    Database *setDb(Database *database);

    QString &getName();
    QString &setName(QString &name);

    int edit(QTableWidgetItem *item = nullptr, QTableWidget *table = nullptr);
    int del(QTableWidgetItem *item);

    QString getCreate();
    bool details(QString &stmt, QSqlQuery &q);

    void setDefaults();
};

#endif // ENTRY_H
