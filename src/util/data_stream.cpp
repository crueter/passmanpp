#include "data_stream.h"

DataStream::DataStream(std::string path, std::fstream::openmode mode)
{
    stream = std::ofstream(path, mode);
    stream.seekp(0);
}

DataStream &DataStream::operator<<(uint8_t val) {
    stream.put(val);
    return *this;
}

DataStream &DataStream::operator<<(uint16_t val) {
    stream.put(val >> 8);
    stream.put(val & 0xFF);
    return *this;
}

DataStream &DataStream::operator<<(int val) {
    stream.put(val);
    return *this;
}

DataStream &DataStream::operator<<(bool val) {
    stream.put(val);
    return *this;
}

DataStream &DataStream::operator<<(const char *val) {
    stream << val;
    return *this;
}

DataStream &DataStream::operator<<(VectorUnion val) {
    for (unsigned long i = 0; i < val.size(); ++i) {
        stream.put(secvec(val)[i]);
    }
    return *this;
}

void DataStream::finish() {
    stream.flush();
    stream.close();
}
