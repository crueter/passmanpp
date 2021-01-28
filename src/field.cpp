#include "field.hpp"

const QString &Field::name() {
    return m_name;
}

const QString &Field::setName(const QString &name) {
    this->m_name = name;
    return name;
}

const QString Field::lowerName() {
    return m_name.toLower();
}

const VectorUnion &Field::data() {
    return m_data;
}

const VectorUnion &Field::setData(const VectorUnion &data) {
    this->m_data = data;
    return data;
}

QString Field::dataStr() {
    return this->m_data.asQStr();
}

QMetaType::Type Field::type() {
    return m_type;
}

QMetaType::Type Field::setType(const QMetaType::Type type) {
    this->m_type = type;
    return type;
}

bool Field::isName() {
    return lowerName() == "name";
}

bool Field::isPass() {
    return lowerName() == "password";
}

bool Field::isMultiLine() {
    return m_type == QMetaType::QByteArray;
}
