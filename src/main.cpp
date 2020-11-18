#include <QApplication>
#include <QListWidgetItem>
#include <QMessageBox>

#include "file_handler.h"
#include "security.h"
#include "manager.h"

std::string PASSMAN_VERSION = "1.3.0";
std::string BOTAN_VERSION = std::to_string(BOTAN_VERSION_MAJOR) + "." + std::to_string(BOTAN_VERSION_MINOR) + "-" + std::to_string(BOTAN_VERSION_PATCH);

int main(int argc,  char** argv) {
    QApplication app (argc, argv);
    FileHandler* fh = new FileHandler();
    EntryHandler* eh = new EntryHandler();
    std::string choice, path;
    if (argc == 1)
        while (1) {
            QMessageBox newChoice;
            newChoice.setText(QMessageBox::tr("Would you like to create a new database?"));
            newChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
            int ret = newChoice.exec();

            if (ret == QMessageBox::Yes) {
                path = fh->newLoc();
                create(path);
                break;
            } else if (ret == QMessageBox::No) {
                path = fh->getDb();
                if (!open(path)) {
                    displayErr("Invalid database file, see previous errors. Aborting.");
                    continue;
                }
                break;
            }
        }
    else {
        bool o = open(argv[1]);
        if (!o) return 1;
        path = std::experimental::filesystem::v1::canonical(argv[1]);
    }
    std::cout << "You may find the source code at https://github.com/binex-dsk/passmanpp." << std::endl << "Type help for available commands." << std::endl;
    while (1) {
        std::cout << "passman> ";
        std::getline(std::cin, choice);

        if (choice == "help")
            std::cout << "edit: add, modify, and delete entries" << std::endl << "tips: tips for password management" << std::endl << "info: some info on passman++" << std::endl << "backup: backup your database" << std::endl << "save: save the database" << std::endl << "exit: exit out of the program" << std::endl;
        else if (choice == "edit")
            eh->entryInteract();
        else if (choice == "tips")
            std::cout << "A good password contains:\n"
                         "- At least 8 characters, but more is always preferred"
                         "- No easy-to-guess phrases or common passwords (See https://github.com/danielmiessler/SecLists/tree/master/Passwords/Common-Credentials)\n"
                         "- At least one each of a lowercase and uppercase letter, a number, and a special character, but more is always preferred\n"
                         "- A sequence of characters with no observable pattern (example: things like a1b2c3d4 are generally not preferrable to something like d.Y2/90a)\n"
                         "- Some sort of meaning that you can use to remember it\n"
                         "A bad password contains:\n"
                         "- Less than 8 characters\n"
                         "- Common, easy-to-guess phrases\n"
                         "- Sequences of repeated characters or obvious patterns\n"
                         "- Little variety in the characters\n"
                         "Never, ever share your passwords. Ever. They are the single most important piece of security in almost everything online. A single person getting your password can cause it to be shared all over the internet, potentially leaking sensitive info.\n"
                         "If you can't think of a good password, hundreds of tools online can help you with that, including on here.\n"
                         "Change your password often. Leaks occur often. Remember to occasionally check https://haveibeenpwned.com/ and enter your email to see if your password may have been leaked.\n"
                         "Do not use the same password for everything. At the very most, use a password on 2 different sites. If someone gets your password and you use the same one for everything, then your accounts will likely be compromised and sensitive info could be leaked.\n"
                         "Do not store your passwords in an easy-to-find location. Either use a password manager like this one, or store it in a place nobody can find. Never just try to \"remember\" them, either--your memory is volatile. A password manager is far better at remembering things than you ever will be.\n"
                         "Lock your computer and phone when not using them, especially if you store passwords on it." << std::endl;
        else if (choice == "info")
            std::cout << "passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help me learn C++.\n"
                         "Project repository: https://github.com/binex-dsk/passmanpp\n"
                         "FOSS Libraries used: Botan, Qt, libsodium\n"
                         "Botan version: " << BOTAN_VERSION << "\n"
                         "Current version: " << PASSMAN_VERSION << std::endl;
        else if (choice == "save")
            save(path);
        else if (choice == "backup") {
            int br = fh->backup(path);
            if (br == 3)
                displayErr("Invalid backup location.");
            else if (br == 17)
                displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
            else
                displayErr("Database backed up successfully.");
        }
        else if (choice == "exit") {
            if (modified)
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
