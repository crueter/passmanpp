#ifndef DATABASE_H
#define DATABASE_H
#include <botan/compression.h>
#include <botan/pwdhash.h>
#include <botan/hash.h>
#include <botan/cipher_mode.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>

#include "stringutil.h"
#include "constants.h"

Botan::secure_vector<uint8_t> toVec(std::string str);
void showMessage(std::string msg);

class Database
{
public:
    Database();

    std::string getPw(std::string password);
    bool showErr(std::string msg);
    bool parse();

    void encrypt(std::string password);
    std::string decrypt(std::string txt = "", std::string password = "");
    bool save(std::string password = "");
    bool convert();

    bool verify(std::string mpass);

    bool keyFile;
    bool modified = false;

    unsigned int checksum;
    unsigned int deriv;
    unsigned int hash;
    unsigned int hashIters;
    unsigned int encryption;
    unsigned int version = MAX_SUPPORTED_VERSION_NUMBER;

    Botan::secure_vector<uint8_t> iv;
    Botan::secure_vector<uint8_t> uuid;
    Botan::secure_vector<uint8_t> data;

    std::string name;
    std::string desc;

    unsigned int ivLen;
    unsigned int uuidLen;
    unsigned int nameLen;
    unsigned int descLen;

    std::string stList;
    std::string path;
    std::string keyFilePath;
};

#endif // DATABASE_H
