#ifndef DATASTREAM_H
#define DATASTREAM_H
#include "extra.h"
#include "vector_union.h"

class DataStream
{
public:
    DataStream(std::string path, std::fstream::openmode mode);

    DataStream &operator<<(uint8_t val);
    DataStream &operator<<(uint16_t val);
    DataStream &operator<<(int val);
    DataStream &operator<<(bool val);
    DataStream &operator<<(const char *val);
    DataStream &operator<<(VectorUnion val);

    void finish();

    std::ofstream stream;
};

#endif // DATASTREAM_H
