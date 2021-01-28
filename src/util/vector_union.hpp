#ifndef VECTORUNION_H
#define VECTORUNION_H
#include "extra.hpp"

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
    VectorUnion(const secvec &data);
    VectorUnion(const std::vector<uint8_t> &data);
    VectorUnion(const QVariant &data);
    VectorUnion(const bool data);
    VectorUnion(const double data);

    QString asQStr() const;
    std::string asStdStr() const;
    QVariant asQVariant() const;

    VectorUnion encoded() const;
    VectorUnion decoded() const;

    explicit operator const char*() const { return reinterpret_cast<const char *>(this->data()); };
    explicit operator bool() const { return this->asQVariant().toBool(); };
    explicit operator double() const { return this->asQVariant().toDouble(); };

    inline VectorUnion &operator+=(QString s) {
        QString t_str = this->asQStr();
        t_str.append(s);
        *this = t_str;
        return *this;
    };
};

#endif // VECTORUNION_H
