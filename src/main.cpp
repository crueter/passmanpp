#include <QApplication>
#include <QAbstractButton>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>

#include "util/extra.h"
#include "util/sql.h"

bool debug = false;
bool verbose = false;

bool choiceHandle(std::string choice, Database *db) {
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
        QGridLayout *layout = new QGridLayout;
        layout->setContentsMargins(0, 0, 0, 0);

        QDialog *openDiag = new QDialog;
        openDiag->resize(600, 300);

        QPushButton *btnCreate = new QPushButton(tr("Create new database"));
        QObject::connect(btnCreate, &QPushButton::clicked, [create, openDiag]() mutable {
            openDiag->accept();
            create();
        });

        QPushButton *btnOpen = new QPushButton(tr("Open existing database"));
        QObject::connect(btnOpen, &QPushButton::clicked, [open, openDiag]() mutable {
            openDiag->accept();
            open(getDb());
        });

        QLabel *label = new QLabel(tr("Welcome to passman++ " + PASSMAN_VERSION));
        QFont font;
        font.setPointSize(16);
        font.setBold(true);
        label->setFont(font);

        layout->addWidget(btnCreate, 3, 0);
        layout->addWidget(btnOpen, 2, 0);

        label->setLayoutDirection(Qt::LeftToRight);
        label->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

        layout->addWidget(label, 1, 0);

        openDiag->setLayout(layout);
        int ret = openDiag->exec();

        if (ret == QDialog::Rejected) {
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
                choiceHandle(arg.toStdString(), db);
            }
        }
    }

    if (createNew) {
        create(path);
    }

    std::string choice;

    while (1) {
        std::cout << "passman> ";
        std::getline(std::cin, choice);

        if (choice == "") {
            continue;
        }

        choiceHandle(choice, db);
    }
    return app.exec();
}
