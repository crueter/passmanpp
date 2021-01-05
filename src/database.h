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

#include "constants.h"
class Entry;

void showMessage(const QString &msg);
std::string getCS(uint8_t cs, uint8_t encr);

class Database
{
    QList<Entry *> entries;
public:
    Database();

    secvec getPw(QString password);
    bool showErr(QString msg);
    bool parse();

    void encrypt();
    QString decrypt(const QString &txt = "", bool convert = false);
    bool config(bool create = true);

    bool open();
    int backup();

    bool save();
    bool convert();

    int verify(const QString &mpass, bool convert = false);
    void get();

    int add(QTableWidget *table);
    int edit();

    bool saveSt(bool exec = true);

    Entry *entryNamed(QString &name);
    void addEntry(Entry *entry);
    bool removeEntry(Entry *entry);
    int entryLength();
    QList<Entry *> &getEntries();
    void setEntries(QList<Entry *> entries);

    template <typename Func>
    QAction *addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func);

    bool keyFile = false;
    bool modified = false;

    uint8_t checksum = 0;
    uint8_t hash = 0;
    uint8_t hashIters = 8;
    uint8_t encryption = 0;
    uint8_t version = MAX_SUPPORTED_VERSION_NUMBER;

    uint8_t ivLen = 12;

    uint16_t memoryUsage = 64;
    uint8_t clearSecs = 15;

    bool compress = true;

    secvec iv;
    secvec data;

    QString name = "None";
    QString desc = "None";

    QString path;
    QString keyFilePath;

    secvec stList;
    secvec passw;
};

#endif // DATABASE_H
