#ifndef FIELD_H
#define FIELD_H
#include <QVariant>

#include "util/vector_union.hpp"

// Class that wraps around an entry data field.
class Field
{
    QString m_name;
    VectorUnion m_data;
    QMetaType::Type m_type;
public:
    Field(const QString &t_name, const VectorUnion &t_data, const QMetaType::Type t_type)
        : m_name(t_name)
        , m_data(t_data)
        , m_type(t_type) {}

    inline const QString &name() {
        return m_name;
    }

    inline const QString &setName(const QString &t_name) {
        this->m_name = t_name;
        return t_name;
    }

    inline const QString lowerName() {
        return this->m_name.toLower();
    }

    inline const VectorUnion &data() {
        return this->m_data;
    }

    inline const VectorUnion &setData(const VectorUnion &t_data) {
        this->m_data = t_data;
        return t_data;
    }

    inline QString dataStr() {
        return this->m_data.asQStr();
    }

    inline QMetaType::Type type() {
        return this->m_type;
    }

    inline QMetaType::Type setType(const QMetaType::Type t_type) {
        this->m_type = t_type;
        return t_type;
    }

    inline bool isName() {
        return this->lowerName() == "name";
    }

    inline bool isPass() {
        return this->lowerName() == "password";
    }

    inline bool isMultiLine() {
        return this->m_type == QMetaType::QByteArray;
    }
};

#endif // FIELD_H
