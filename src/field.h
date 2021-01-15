#ifndef FIELD_H
#define FIELD_H
#include <QVariant>

#include "util/vector_union.h"

class Field
{
    QString _name;
    VectorUnion _data;
    QMetaType::Type _type;
public:
    Field(QString name, VectorUnion data, QMetaType::Type type)
        : _name(name)
        , _data(data)
        , _type(type) {}

    const QString &name();
    QString &setName(QString &name);
    QString lowerName();

    const VectorUnion &data();
    VectorUnion setData(VectorUnion data);
    QString dataStr();

    QMetaType::Type type();
    QMetaType::Type setType(QMetaType::Type type);

    bool isName();
    bool isPass();
    bool isMultiLine();
};

#endif // FIELD_H
