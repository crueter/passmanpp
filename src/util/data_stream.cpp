#include "data_stream.hpp"

DataStream::DataStream(const std::string &path, const std::fstream::openmode mode)
{
    stream = std::ofstream(path, mode);
    stream.seekp(0);
}

DataStream &DataStream::operator<<(const uint8_t val) {
    stream.put(val);
    return *this;
}

DataStream &DataStream::operator<<(const uint16_t val) {
    stream.put(static_cast<char>(val << 8));
    stream.put(static_cast<char>(val & 0xFF));
    return *this;
}

DataStream &DataStream::operator<<(const int val) {
    stream.put(static_cast<char>(val));
    return *this;
}

DataStream &DataStream::operator<<(const bool val) {
    stream.put(val);
    return *this;
}

DataStream &DataStream::operator<<(const char *val) {
    stream << val;
    return *this;
}

DataStream &DataStream::operator<<(const VectorUnion val) {
    for (const int i : range(0, static_cast<int>(val.size()))) {
        stream.put(secvec(val)[i]);
    }
    return *this;
}

void DataStream::finish() {
    stream.flush();
    stream.close();
}
