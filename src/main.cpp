#include <QApplication>
#include <QAbstractButton>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

#include "util/extra.h"
#include "util/sql.h"
#include "gui/welcome_dialog.h"

bool debug = false;
bool verbose = false;

int main(int argc, char** argv) {
    QApplication app (argc, argv);

    if (getenv("PASSMAN_DEBUG")) {
        std::cout << "Debug mode activated. Do NOT use this unless you are testing stuff." << std::endl;
        debug = true;
    }
    if (getenv("PASSMAN_DEBUG")) {
        std::cout << "Verbose mode activated. Do NOT use this unless you are testing stuff." << std::endl;
        verbose = true;
    }

    dbInit();
    Database *db = new Database;
    QString path;

    bool createNew = false;

    auto create = [path, db](QString spath = "") mutable {
            if (spath.isEmpty()) {
                path = newLoc();
                if (path.isEmpty()) {
                    delete db;
                    exit(1);
                }
            } else {
                path = spath;
            }
            db->path = path;
            bool cr = db->config(true);
            if (!cr) {
                delete db;
                exit(1);
            }
            return true;
    };

    auto open = [db](QString spath) {
        db->path = spath;

        if (!db->open()) {
            exit(1);
        }
    };

    if (argc <= 1) {
        WelcomeDialog *di = new WelcomeDialog(db);
        di->init();
        di->setup();

        if (di->show() == QDialog::Rejected) {
            delete db;
            return 1;
        }
    } else {
        for (int i = 1; i < argc; ++i) {
            QString arg(argv[i]);
            if (arg == "new") {
                createNew = true;
            } else if (arg == "help") {
                qDebug() << usage.data();
                delete db;
                return 1;
            } else if (arg == "info") {
                qDebug() << info.data();
                delete db;
                return 1;
            } else if (createNew) {
                path = arg;
            } else {
                open(arg);
            }
        }
    }

    if (createNew) {
        create(path);
    }

    db->edit();
    db->save();

    return 0;
}
