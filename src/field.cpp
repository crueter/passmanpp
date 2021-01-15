#include "field.h"

const QString &Field::name() {
    return _name;
}

QString &Field::setName(QString &name) {
    this->_name = name;
    return name;
}

QString Field::lowerName() {
    return _name.toLower();
}

const VectorUnion &Field::data() {
    return _data;
}

VectorUnion Field::setData(VectorUnion data) {
    this->_data = data;
    return data;
}

QString Field::dataStr() {
    return this->_data;
}

QMetaType::Type Field::type() {
    return _type;
}

QMetaType::Type Field::setType(QMetaType::Type type) {
    this->_type = type;
    return type;
}

bool Field::isName() {
    return lowerName() == "name";
}

bool Field::isPass() {
    return lowerName() == "password";
}

bool Field::isMultiLine() {
    return _type == QMetaType::QByteArray;
}
