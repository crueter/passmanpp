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
    bool saveAs(std::string savePath);
    bool convert();

    bool verify(std::string mpass);

    bool keyFile;
    bool modified = false;

    int checksum;
    int deriv;
    int hash;
    int hashIters;
    int encryption;
    int version = MAX_SUPPORTED_VERSION_NUMBER;

    std::string iv;
    std::string uuid;
    std::string name;
    std::string desc;
    std::string data;

    std::string stList;
    std::string path;
};

#endif // DATABASE_H
