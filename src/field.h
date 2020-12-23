#ifndef FIELD_H
#define FIELD_H
#include <QVariant>

class Field
{
public:
    Field(QString name, QVariant data, QMetaType::Type type);

    QString name;
    QVariant data;
    QMetaType::Type type;
};

#endif // FIELD_H
