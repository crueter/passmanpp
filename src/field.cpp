
#include "field.h"

Field::Field(QString name, QVariant data, QMetaType::Type type) {
    this->name = name;
    this->data = data;
    this->type = type;
}
