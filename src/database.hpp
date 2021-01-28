#ifndef DATABASE_H
#define DATABASE_H

#include <botan/compression.h>
#include <botan/pwdhash.h>
#include <botan/hash.h>
#include <botan/cipher_mode.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <experimental/filesystem>
#include <QTableWidgetItem>

#include "constants.hpp"
#include "util/vector_union.hpp"

class Entry;

void showMessage(const QString &msg);

class Database
{
    QList<Entry *> m_entries;
public:
    Database() {}

    inline std::unique_ptr<Botan::Cipher_Mode> makeEncryptor() {
        return Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);
    }

    inline std::unique_ptr<Botan::Cipher_Mode> makeDecryptor() {
        return Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::DECRYPTION);
    }

    const std::string getCS();

    VectorUnion hashPw(VectorUnion password);
    secvec getPw(VectorUnion password);

    bool showErr(const QString &msg);
    bool parse();

    VectorUnion getKey();

    VectorUnion encryptedData();
    void encrypt();

    VectorUnion decryptData(const VectorUnion t_data, const VectorUnion &mpass, const bool convert = false);
    const QString decrypt(const QString &txt = "", const bool convert = false);

    bool config(const bool create = true);
    bool open();

    int saveAs();
    bool save();

    int verify(const VectorUnion &mpass, const bool convert = false);
    void get();

    int add(QTableWidget *table);
    int edit();
    bool saveSt(const bool exec = true);

    Entry *entryNamed(const QString &ename);
    void addEntry(Entry *entry);
    bool removeEntry(Entry *entry);
    qsizetype entryLength();
    QList<Entry *> &entries();
    void setEntries(QList<Entry *> entries);

    void redrawTable(QTableWidget *table);

    bool keyFile = false;
    bool modified = false;

    uint8_t hmac = 0;
    uint8_t hash = 0;
    uint8_t hashIters = 8;
    uint8_t encryption = 0;
    uint8_t version = maxVersion;

    size_t ivLen = 12;

    uint16_t memoryUsage = 64;
    uint8_t clearSecs = 15;

    bool compress = true;

    VectorUnion iv{};
    VectorUnion data{};

    VectorUnion name = "None";
    VectorUnion desc = "None";

    VectorUnion path = "";
    VectorUnion keyFilePath = "";

    VectorUnion stList = "";
    VectorUnion passw{};
};

#endif // DATABASE_H
