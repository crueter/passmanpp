#include <botan/hex.h>
#include "vector_union.hpp"

VectorUnion::VectorUnion(const QString &data) {
    *this = toVec(data);
}

VectorUnion::VectorUnion(const std::string &data) {
    *this = toVec(data);
}

VectorUnion::VectorUnion(const char *data) {
    *this = QString(data);
}

VectorUnion::VectorUnion(const secvec &data) {
    this->assign(data.begin(), data.end());
}

VectorUnion::VectorUnion(const std::vector<uint8_t> &data) {
    this->operator=(secvec(data.begin(), data.end()));
}

VectorUnion::VectorUnion(const QVariant &data) {
    this->operator=(data.toString());
}

VectorUnion::VectorUnion(const bool data) {
    this->operator=(QVariant(data));
}

VectorUnion::VectorUnion(const double data) {
    this->operator=(QVariant(data));
}

QString VectorUnion::asQStr() const {
    return QString::fromStdString(asStdStr());
}

std::string VectorUnion::asStdStr() const {
    return std::string(this->begin(), this->end());
}

QVariant VectorUnion::asQVariant() const {
    return QVariant(this->asQStr());
}

VectorUnion VectorUnion::encoded() const {
    return Botan::hex_encode(*this);
}

VectorUnion VectorUnion::decoded() const {
    return Botan::hex_decode(this->asStdStr());
}
