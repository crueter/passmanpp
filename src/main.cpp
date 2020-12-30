#include <QApplication>
#include <QAbstractButton>

#include "util/extra.h"
#include "util/sql.h"

bool debug = false;
bool verbose = false;

bool choiceHandle(QString choice, Database *db) {
    if (choice == "help") {
        std::cout << help << std::endl;
    } else if (choice == "edit") {
        db->edit();
        db->saveSt();
    } else if (choice == "tips") {
        std::cout << tips << std::endl;
    } else if (choice == "info") {
        std::cout << info << std::endl;
    } else if (choice == "save") {
        if (!db->save()) {
            std::cerr << "Cancelled." << std::endl;
        }
    } else if (choice == "backup") {
        int br = db->backup();
        if (br == 3) {
            displayErr("Invalid backup location.");
        } else if (br == 17) {
            displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
        } else if (!br) {
            std::cerr << "Cancelled." << std::endl;
        } else {
            std::cout << "Database backed up successfully." << std::endl;
        }
    } else if (choice == "config") {
        if (!db->config(false)) {
            return false;
        }
    } else if (choice == "exit") {
        if (db->modified) {
            std::cout << "Please save your work before leaving." << std::endl;
        } else {
            exit(0);
        }
    } else {
        std::cerr << "Invalid choice. Type help for available commands." << std::endl;
    }
    return true;
}

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
            if (spath == "") {
                path = newLoc();
                if (path == "") {
                    delete db;
                    exit(1);
                }
            } else {
                path = spath;
            }
            db->path = path;
            bool cr = db->config();
            if (!cr) {
                delete db;
                exit(1);
            }
            return true;
    };

    auto open = [db](QString spath) {
        db->path = spath;

        if (!db->open()) {
            qDebug() << "Aborted.";
            exit(1);
        }
    };

    if (argc <= 1) {
        QMessageBox newChoice;
        newChoice.setText(tr("Create new database, or open existing?"));
        newChoice.setStandardButtons(QMessageBox::Save | QMessageBox::Open | QMessageBox::Cancel);
        newChoice.button(QMessageBox::Save)->setText(tr("New"));
        int ret = newChoice.exec();

        switch (ret) {
            case QMessageBox::Save: {
                create();
                break;
            }
            case QMessageBox::Open: {
                open(getDb());
                break;
            } default: {
                delete db;
                return 1;
                break;
            }
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
            } else if (arg == "tips") {
                qDebug() << tips.data();
                delete db;
                return 1;
            } else if (arg == "info") {
                qDebug() << info.data();
                delete db;
                return 1;
            } else if (createNew) {
                path = arg;
            } else if (!choices.contains(arg)) {
                open(arg);
            } else {
                choiceHandle(arg, db);
            }
        }
    }

    while (1) {
        std::cout << "passman> ";
        QTextStream in(stdin);
        QString choice = in.readLine();

        in.flush();
        in.reset();

        choiceHandle(choice, db);
    }
    return app.exec();
}
