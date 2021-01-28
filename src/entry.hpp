#ifndef ENTRY_H
#define ENTRY_H
#include <QTableWidgetItem>

#include "util/sql.hpp"
#include "field.hpp"
#include "database.hpp"

class Entry
{
    QList<Field *> m_fields;
    Database *m_database;
    QString m_name;
public:
    Entry() = default;
    Entry(QList<Field *> fields, Database *tdb);

    void addField(Field *field);
    bool removeField(Field *field);
    qsizetype indexOf(Field *field);
    Field *fieldNamed(QString name);
    Field *fieldAt(const int index);

    const QList<Field *> &fields();
    QList<Field *> &setFields(QList<Field *> &fields);
    qsizetype fieldLength();

    const Database *database();
    Database *setDb(Database *database);

    const QString &name();
    QString &setName(QString &name);

    int edit(QTableWidgetItem *item = nullptr, QTableWidget *table = nullptr);
    bool del(QTableWidgetItem *item);

    QString getCreate();
    void setDefaults();
};

#endif // ENTRY_H
