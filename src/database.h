#ifndef DATABASE_H
#define DATABASE_H
#include <botan/compression.h>
#include <botan/pwdhash.h>
#include <botan/hash.h>
#include <botan/cipher_mode.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <experimental/filesystem>

#include "constants.h"

extern std::string kfp;
void showMessage(std::string msg);
std::string getCS(uint8_t cs, uint8_t encr);

class Database
{
public:
    Database();

    std::string getPw(std::string password);
    bool showErr(std::string msg);
    bool parse();

    void encrypt(std::string password);
    std::string decrypt(std::string txt = "", std::string password = "");
    bool config(bool create = true);

    bool open();
    int backup();

    bool save(std::string password = "");
    bool convert();

    int verify(std::string mpass);

    bool keyFile;
    bool modified = false;

    uint8_t checksum;
    uint8_t deriv;
    uint8_t hash;
    uint8_t hashIters;
    uint8_t encryption;
    uint8_t version = MAX_SUPPORTED_VERSION_NUMBER;

    uint8_t ivLen;

    Botan::secure_vector<uint8_t> iv;
    Botan::secure_vector<uint8_t> data;

    std::string name;
    std::string desc;

    std::string stList;
    std::string path;
    std::string keyFilePath;
};

#endif // DATABASE_H
