#ifndef KDF_HPP
#define KDF_HPP
#include <botan/cipher_mode.h>
#include <botan/pwdhash.h>

#include "vector_union.hpp"
#include "../constants.hpp"

class Database;

class KDF
{
    uint16_t m_i1;
    uint16_t m_i2;
    uint16_t m_i3;

    uint8_t m_hmacFunction;
    uint8_t m_hashFunction;
    uint8_t m_encryptionFunction;

    VectorUnion m_seed;
    VectorUnion m_keyFile;
public:
    KDF(const QVariantMap &p);
    KDF() = default;
    virtual ~KDF() = default;

    bool setParams(const QVariantMap &p);

    uint16_t i1();
    void setI1(uint16_t t_i1);
    uint16_t i2();
    void setI2(uint16_t t_i2);
    uint16_t i3();
    void setI3(uint16_t t_i3);
    uint16_t rounds();
    uint16_t memoryUsage();
    uint16_t parallelism();

    uint8_t hmacFunction();
    bool setHmacFunction(uint8_t t_hmacFunction);
    uint8_t hashFunction();
    bool setHashFunction(uint8_t t_hashFunction);
    uint8_t encryptionFunction();
    bool setEncryptionFunction(uint8_t t_encryptionFunction);

    VectorUnion seed();
    bool setSeed(VectorUnion t_seed);
    VectorUnion keyFile();
    VectorUnion readKeyFile();
    bool setKeyFile(VectorUnion t_keyFile);

    std::unique_ptr<Botan::Cipher_Mode> makeEncryptor(uint8_t t_encryptionFunction = 63);
    std::unique_ptr<Botan::Cipher_Mode> makeDecryptor(uint8_t t_encryptionFunction = 63);
    std::unique_ptr<Botan::PasswordHash> makeDerivation(uint8_t t_hmacFunction = 63);
    std::unique_ptr<Botan::PasswordHash> makeHasher(uint8_t t_hashFunction = 63);

    VectorUnion transform(VectorUnion t_data, VectorUnion t_seed = {});
    int benchmark(const int t_msec);

    QString toString();
};

#endif // KDF_HPP
