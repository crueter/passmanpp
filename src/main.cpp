#include <QApplication>

#include "manager.h"
#include "file_handler.h"
#include "entry_handler.h"
#include "database.h"
#include "constants.h"

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
            newChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            int ret = newChoice.exec();

            if (ret == QMessageBox::Yes) {
                path = fh->newLoc();
                db.path = path;
                eh->create(db);
                break;
            } else if (ret == QMessageBox::No) {
                path = fh->getDb();
                db.path = path;
                if (!open(db)) {
                    std::cout << "Aborted." << std::endl;
                    continue;
                }
                break;
            }
        }
    } else {
        db.path = argv[1];
        bool o = open(db);

        if (!o) return 1;
        path = std::experimental::filesystem::v1::canonical(argv[1]);
        db.path = path;
    }

    db.parse();

    std::cout << "You may find the source code at https://github.com/binex-dsk/passmanpp." << std::endl << "Type help for available commands." << std::endl;
    while (1) {
        std::cout << "passman> ";
        std::getline(std::cin, choice);

        if (choice == "help")
            std::cout << "edit: add, modify, and delete entries" << std::endl << "tips: tips for password management" << std::endl << "info: some info on passman++" << std::endl << "backup: backup your database" << std::endl << "save: save the database" << std::endl << "exit: exit out of the program" << std::endl;
        else if (choice == "edit")
            eh->entryInteract(db);
        else if (choice == "tips")
            std::cout << tips << std::endl;
        else if (choice == "info")
            std::cout << info << std::endl;
        else if (choice == "save")
            db.save();
        else if (choice == "backup") {
            int br = fh->backup(db, path);
            if (br == 3) {
                displayErr("Invalid backup location.");
            } else if (br == 17) {
                displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
            } else {
                std::cout << "Database backed up successfully." << std::endl;
            }
        }
        else if (choice == "exit") {
            if (db.modified)
                std::cout << "Please save your work before leaving." << std::endl;
            else {
                std::cout << "Thanks for using passman++." << std::endl;
                return 0;
            }
        } else
            std::cout << "Invalid choice. Type help for available commands." << std::endl;
    }
    return app.exec();
}
