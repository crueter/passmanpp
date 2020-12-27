
#include "field.h"

Field::Field(QString name, QVariant data, QMetaType::Type type) {
    this->name = name;
    this->data = data;
    this->type = type;
}

QString &Field::getName() {
    return name;
}

QString &Field::setName(QString &name) {
    this->name = name;
    return name;
}

QString Field::lowerName() {
    return name.toLower();
}

QVariant &Field::getData() {
    return data;
}

QVariant Field::setData(QVariant data) {
    this->data = data;
    return data;
}

QString Field::dataStr() {
    return this->data.toString();
}

QMetaType::Type Field::getType() {
    return type;
}

QMetaType::Type Field::setType(QMetaType::Type type) {
    this->type = type;
    return type;
}

bool Field::isName() {
    return lowerName() == "name";
}

bool Field::isPass() {
    return lowerName() == "password";
}

bool Field::isMultiLine() {
    return type == QMetaType::QByteArray;
}
