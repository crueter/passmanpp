#include <QApplication>

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
    QString choice, path;

    if (argc <= 1) {
        while (1) {
            QMessageBox newChoice;
            newChoice.setText(QWidget::tr("Would you like to create a new database?"));
            newChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            int ret = newChoice.exec();

            if (ret == QMessageBox::Yes) {
                path = newLoc();
                if (path == "") {
                    return 1;
                }
                db->path = path;
                bool cr = db->config();
                if (!cr) {
                    return 1;
                }
                break;
            } else if (ret == QMessageBox::No) {
                path = getDb();
                db->path = path;
                if (!db->open()) {
                    std::cout << "Aborted." << std::endl;
                    continue;
                }
                break;
            } else if (ret == QMessageBox::Cancel) {
                return 1;
            }
        }
    } else if (QString(argv[1]) == "new") {
        if (argc < 3) {
            path = newLoc();
            if (path == "") {
                return 1;
            }
        } else {
            path = QString(argv[2]);
        }
        db->path = path;
        bool cr = db->config();
        if (!cr) {
            return 1;
        }
    } else if (QString(argv[1]) == "help") {
        std::cout << usage << std::endl;
        return 1;
    } else {
        db->path = argv[1];
        db->parse();
        bool o = db->open();

        if (!o) {
            return 1;
        }

        path = argv[1];
        db->path = path;
    }

    if (!db->parse()) {
        return 1;
    }

    if (argc >= 3 && QString(argv[1]) != "new") {
        choiceHandle(argv[2], db);
    }

    std::cout << welcomeMessage << std::endl;
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
