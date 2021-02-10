#ifndef DATASTREAM_H
#define DATASTREAM_H
#include "extra.hpp"
#include "vector_union.hpp"

/* Utility class for writing a couple extra types to an std::ofstream.
 * Prefer over direct operator overloads when overloading already-existing overloads. */
class DataStream
{
public:
    inline DataStream(const std::string &path, const std::fstream::openmode mode)
        : stream(path, mode)
    {
        stream.seekp(0);
    }

    inline DataStream &operator<<(const uint8_t val) {
        stream.put(val);
        return *this;
    }

    inline DataStream &operator<<(const uint16_t val) {
        stream.put(static_cast<char>(val << 8));
        stream.put(static_cast<char>(val & 0xFF));
        return *this;
    }

    inline DataStream &operator<<(const int val) {
        stream.put(static_cast<char>(val));
        return *this;
    }

    inline DataStream &operator<<(const bool val) {
        stream.put(val);
        return *this;
    }

    inline DataStream &operator<<(const char *val) {
        stream << val;
        return *this;
    }

    inline DataStream &operator<<(const VectorUnion val) {
        for (const int i : range(0, static_cast<int>(val.size()))) {
            stream.put(secvec(val)[i]);
        }
        return *this;
    }

    // Flush and close the stream.
    inline void finish() {
        stream.flush();
        stream.close();
    }

    std::ofstream stream;
};

#endif // DATASTREAM_H
