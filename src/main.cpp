#include <QApplication>

#include "manager.h"

int main(int argc,  char** argv) {
    QApplication app (argc, argv);
    FileHandler* fh = new FileHandler;
    EntryHandler* eh = new EntryHandler;
    Database db;
    std::string choice, path;

    if (argc <= 1) {
        while (1) {
            QMessageBox newChoice;
            newChoice.setText(QWidget::tr("Would you like to create a new database?"));
            newChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel);
            int ret = newChoice.exec();

            if (ret == QMessageBox::Yes) {
                path = fh->newLoc();
                if (path == "") {
                    return 1;
                }
                db.path = path;
                bool cr = eh->create(db);
                if (!cr) {
                    return 1;
                }
                break;
            } else if (ret == QMessageBox::No) {
                path = fh->getDb();
                db.path = path;
                if (!fh->open(db)) {
                    std::cout << "Aborted." << std::endl;
                    continue;
                }
                break;
            } else if (ret == QMessageBox::Cancel) {
                return 1;
            }
        }
    } else if (std::string(argv[1]) == "new") {
        path = fh->newLoc();
        if (path == "") {
            return 1;
        }
        db.path = path;
        bool cr = eh->create(db);
        if (!cr) {
            return 1;
        }
    } else if (std::string(argv[1]) == "help") {
        std::cout << usage << std::endl;
        return 1;
    } else {
        db.path = argv[1];
        db.parse();
        bool o = fh->open(db);

        if (!o) {
            return 1;
        }
        path = std::experimental::filesystem::v1::canonical(argv[1]);
        db.path = path;
    }

    db.parse();

    if (argc >= 3) {
        choiceHandle(argv[2], eh, fh, db, path);
    }

    std::cout << welcomeMessage << std::endl;
    while (1) {
        std::cout << "passman> ";
        std::getline(std::cin, choice);

        choiceHandle(choice, eh, fh, db, path);
    }
    return app.exec();
}
