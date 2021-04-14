#ifndef VECTORUNION_H
#define VECTORUNION_H
#include <botan/hex.h>

#include "extra.hpp"

/* Helper class which stores a Botan::secure_vector<uint8_t>.
 * Primary use is secure storage of many different types, allowing for assignment and conversion to/from these types. Supported types include:
 * QString, std::string, const char *, secure vector, QVariant, bool, and double */
class VectorUnion : public secvec
{
public:
    inline VectorUnion() = default;
    virtual ~VectorUnion() = default;
    inline VectorUnion(const QString &data) {
        *this = data.toStdString();
    }

    inline VectorUnion(const std::string &data) {
        *this = secvec(data.begin(), data.end());
    }

    inline VectorUnion(const char *data, const int length = 0) {
        if (length == 0) {
            *this = QString(data);
        } else {
            *this = std::string(data, length);
        }
    }

    inline VectorUnion(const secvec &data) {
        this->assign(data.begin(), data.end());
    }

    inline VectorUnion(const std::vector<uint8_t> &data) {
        this->operator=(secvec(data.begin(), data.end()));
    }

    inline VectorUnion(const QVariant &data) {
        this->operator=(data.toString());
    }

    inline VectorUnion(const bool data) {
        this->operator=(QVariant(data));
    }

    inline VectorUnion(const double data) {
        this->operator=(QVariant(data));
    }

    inline VectorUnion(const QByteArray &data) {
        *this = VectorUnion(data.constData(), static_cast<int>(data.size()));
    }

    inline const char *asConstChar() const {
        return reinterpret_cast<const char *>(this->data());
    }

    inline QString asQStr() const {
        return QString::fromStdString(asStdStr());
    }

    inline std::string asStdStr() const {
        return std::string(this->begin(), this->end());
    }

    inline QVariant asQVariant() const {
        return QVariant(this->data());
    }

    inline QByteArray asQByteArray() const {
        return QByteArray(this->asConstChar());
    }

    inline VectorUnion encoded() const {
        return Botan::hex_encode(*this);
    }

    inline VectorUnion decoded() const {
        return Botan::hex_decode(this->asStdStr());
    }

    explicit inline operator bool() const {
        return this->asQVariant().toBool();
    };

    explicit inline operator double() const {
        return this->asQVariant().toDouble();
    };

    inline VectorUnion &operator+=(QString s) {
        QString t_str = this->asQStr();
        t_str.append(s);
        *this = t_str;
        return *this;
    };
};

#endif // VECTORUNION_H
