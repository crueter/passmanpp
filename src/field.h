#ifndef FIELD_H
#define FIELD_H
#include <QVariant>

class Field
{
    QString name;
    QVariant data;
    QMetaType::Type type;
public:
    Field(QString name, QVariant data, QMetaType::Type type);

    QString &getName();
    QString &setName(QString &name);
    QString lowerName();

    QVariant &getData();
    QVariant setData(QVariant data);
    QString dataStr();

    QMetaType::Type getType();
    QMetaType::Type setType(QMetaType::Type type);

    bool isName();
    bool isPass();
    bool isMultiLine();
};

#endif // FIELD_H
