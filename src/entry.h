#ifndef ENTRY_H
#define ENTRY_H
#include <QTableWidgetItem>

#include "util/sql.h"
#include "field.h"
#include "database.h"

class Entry
{
    QList<Field *> _fields;
    Database *_database;
    QString _name;
public:
    Entry(QList<Field *> fields, Database *tdb);

    void addField(Field *field);
    bool removeField(Field *field);
    int indexOf(Field *field);
    Field *fieldNamed(QString name);
    Field *fieldAt(int index);

    const QList<Field *> &fields();
    QList<Field *> &setFields(QList<Field *> &fields);
    int fieldLength();

    const Database *database();
    Database *setDb(Database *database);

    const QString &name();
    QString &setName(QString &name);

    int edit(QTableWidgetItem *item = nullptr, QTableWidget *table = nullptr);
    bool del(QTableWidgetItem *item);

    QString getCreate();
    bool details(QString &stmt, QSqlQuery &q);

    void setDefaults();
};

#endif // ENTRY_H
