#ifndef DATABASE_H
#define DATABASE_H

#include <botan/compression.h>
#include <botan/pwdhash.h>
#include <botan/hash.h>
#include <botan/cipher_mode.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <QTableWidgetItem>

#include "constants.hpp"
#include "util/vector_union.hpp"
#include "gui/database_window.hpp"
#include "gui/config_dialog.hpp"

class Entry;

void showMessage(const QString &msg);

// TODO: getters and setters for variables

// Drives all operations related to database access.
class Database
{
    QList<Entry *> m_entries;
public:
    Database() {}

    inline std::unique_ptr<Botan::Cipher_Mode> makeEncryptor() {
        return Botan::Cipher_Mode::create(Constants::encryptionMatch.at(this->encryption), Botan::ENCRYPTION);
    }

    inline std::unique_ptr<Botan::Cipher_Mode> makeDecryptor() {
        return Botan::Cipher_Mode::create(Constants::encryptionMatch.at(this->encryption), Botan::DECRYPTION);
    }

    // Generates a checksum string.
    inline const std::string getCS() {
        auto enc = this->makeEncryptor();
        std::string hmacChoice(Constants::hmacMatch[this->hmac]);

        if (hmacChoice != "SHA-512") {
            hmacChoice += '(' + std::to_string(enc->maximum_keylength() * 8) + ')';
        }

        return hmacChoice;
    }

    // The main window
    inline int edit() {
        DatabaseWindow *di = new DatabaseWindow(this);
        di->setup();
        return di->exec();
    }

    // Grabs the contents of the keyfile.
    inline VectorUnion getKey() {
        QFile kf(keyFilePath.asQStr());
        kf.open(QIODevice::ReadOnly);
        QTextStream key(&kf);

        return key.readAll();
    }

    inline bool save() {
        this->encrypt();

        this->modified = false;
        return true;
    }

    inline bool showErr(const QString &t_msg) {
        displayErr("Error: database file is corrupt or invalid.\nDetails: " + t_msg);
        return false;
    }

    // Opens the database configuration dialog
    inline bool config(const bool create = true) {
        ConfigDialog *di = new ConfigDialog(this, create);
        di->setup();
        return di->show();
    }

    inline void addEntry(Entry *entry) {
        this->m_entries.emplaceBack(entry);
    }

    inline bool removeEntry(Entry *entry) {
        return this->m_entries.removeOne(entry);
    }

    inline qsizetype entryLength() {
        return this->m_entries.length();
    }

    inline QList<Entry *> &entries() {
        return this->m_entries;
    }

    inline void setEntries(QList<Entry *> t_entries) {
        this->m_entries = t_entries;
    }

    Entry *entryNamed(const QString &t_name);
    Entry *entryWithPassword(const QString &t_pass);

    void get();
    bool saveSt();

    int add(QTableWidget *table);

    VectorUnion hashPw(VectorUnion password);
    secvec getPw(VectorUnion password);

    VectorUnion encryptedData();
    void encrypt();

    VectorUnion decryptData(const VectorUnion t_data, const VectorUnion &mpass, const bool convert = false);
    int verify(const VectorUnion &mpass, const bool convert = false);
    const QString decrypt(const QString &txt = "", const bool convert = false);

    bool parse();

    bool open();
    int saveAs();

    void redrawTable(QTableWidget *table);

    bool keyFile = false;
    bool modified = false;

    uint8_t hmac = 0;
    uint8_t hash = 0;
    uint8_t hashIters = 8;
    uint8_t encryption = 0;
    uint8_t version = Constants::maxVersion;

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
