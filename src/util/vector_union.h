#ifndef VECTORUNION_H
#define VECTORUNION_H
#include "extra.h"

/* This is probably one of the most important classes in the ENTIRE project.
 * It combines QString, std::string, const char*, QVariant, bool, and double effectively all into one class,
 * entirely controlled by a secure vector. By storing all types of data in this class,
 * all of their data is put into locked memory, meaning memory dumps by unprivileged users, or swapping, can not see this memory,
 * while also allowing the data to freely convert to other types. */
class VectorUnion : public secvec
{
public:
    VectorUnion() {}
    VectorUnion(const QString &data);
    VectorUnion(const std::string &data);
    VectorUnion(const char *data);
    VectorUnion(secvec data);
    VectorUnion(std::vector<uint8_t> data);
    VectorUnion(const QVariant &data);
    VectorUnion(bool data);
    VectorUnion(double data);

    QString asQStr() const;
    std::string asStdStr() const;
    QVariant asQVariant() const;

    VectorUnion encoded();
    VectorUnion decoded();

    using secvec::data;
    using secvec::size;
    using secvec::begin;
    using secvec::end;

    operator const char*() const { return reinterpret_cast<const char *>(this->data()); };
    operator QString() const { return this->asQStr(); };
    operator const std::string() const { return this->asStdStr(); };
    operator QVariant() const { return this->asQVariant(); };
    operator bool() const { return this->asQVariant().toBool(); };
    operator double() const { return this->asQVariant().toDouble(); };

    inline VectorUnion &operator+=(QString s) {
        QString _str = *this;
        _str.append(s);
        *this = _str;
        return *this;
    };
};

#endif // VECTORUNION_H
