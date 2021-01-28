#ifndef FIELD_H
#define FIELD_H
#include <QVariant>

#include "util/vector_union.hpp"

class Field
{
    QString m_name;
    VectorUnion m_data;
    QMetaType::Type m_type;
public:
    Field(const QString &name, const VectorUnion &data, const QMetaType::Type type)
        : m_name(name)
        , m_data(data)
        , m_type(type) {}

    const QString &name();
    const QString &setName(const QString &name);
    const QString lowerName();

    const VectorUnion &data();
    const VectorUnion &setData(const VectorUnion &data);
    QString dataStr();

    QMetaType::Type type();
    QMetaType::Type setType(const QMetaType::Type type);

    bool isName();
    bool isPass();
    bool isMultiLine();
};

#endif // FIELD_H
