#ifndef DATABASE_H
#define DATABASE_H

#include <botan/compression.h>
#include <botan/pwdhash.h>
#include <botan/hash.h>
#include <botan/cipher_mode.h>
#include <botan/hex.h>
#include <QTableWidgetItem>

#include <passman/pdpp_database.hpp>
#include <passman/pdpp_entry.hpp>

#include "gui/database_main_widget.hpp"
#include "gui/mainwindow.hpp"
#include "gui/config_widget.hpp"
#include "util.hpp"

// TODO: getters and setters for variables

// Drives all operations related to database access.
class Database : public passman::PDPPDatabase
{
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

    void add();

    int saveAsPrompt();

    passman::KDF *makeKdf(uint8_t t_hmac = 63, uint8_t t_hash = 63, uint8_t t_encryption = 63, passman::VectorUnion t_seed = {}, passman::VectorUnion t_keyFile = {}, uint8_t t_hashIters = 0, uint16_t t_memoryUsage = 0);

    MainWindow *window;
    DatabaseWidget *widget;
};

#endif // DATABASE_H
