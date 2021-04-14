#ifndef DATABASE_H
#define DATABASE_H

#include <botan/compression.h>
#include <botan/pwdhash.h>
#include <botan/hash.h>
#include <botan/cipher_mode.h>
#include <botan/hex.h>
#include <QTableWidgetItem>

#include "constants.hpp"
#include "util/vector_union.hpp"
#include "util/kdf.hpp"
#include "gui/database_main_widget.hpp"
#include "gui/mainwindow.hpp"
#include "gui/config_widget.hpp"

class Entry;

// TODO: getters and setters for variables

// Drives all operations related to database access.
class Database
{
    QList<Entry *> m_entries;
public:
    Database(MainWindow *t_window = nullptr)
        : window(t_window)
        , widget(new DatabaseWidget(this))
    {
        widget->setup();
    }

    // The main window
    inline void edit() {
        widget->show();
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
    inline void config(const bool create = true) {
        ConfigWidget *di = new ConfigWidget(this, create);
        di->setup();
        di->show();
    }

    inline void addEntry(Entry *entry) {
        this->m_entries.emplaceBack(entry);
        this->modified = true;
    }

    inline bool removeEntry(Entry *entry) {
        return this->m_entries.removeOne(entry);
        this->modified = true;
    }

    inline qsizetype entryLength() {
        return this->m_entries.length();
    }

    inline QList<Entry *> &entries() {
        return this->m_entries;
    }

    inline void setEntries(QList<Entry *> t_entries) {
        this->m_entries = t_entries;
        this->modified = true;
    }

    Entry *entryNamed(const QString &t_name);
    Entry *entryWithPassword(const QString &t_pass);

    void get();
    bool saveSt();

    void add();

    VectorUnion encryptedData();
    void encrypt();

    std::pair<VectorUnion, int> decryptData(const VectorUnion t_data, const VectorUnion &mpass, const bool convert = false);
    int verify(const VectorUnion &mpass, const bool convert = false);
    bool decrypt(PasswordOptionsFlag options = PasswordOptions());

    bool parse();

    bool open();
    int saveAs();

    KDF *makeKdf(uint8_t t_hmac = 63, uint8_t t_hash = 63, uint8_t t_encryption = 63, VectorUnion t_seed = {}, VectorUnion t_keyFile = {}, uint8_t t_hashIters = 0, uint16_t t_memoryUsage = 0);

    MainWindow *window;
    DatabaseWidget *widget;

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
