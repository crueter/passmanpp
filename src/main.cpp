#include <QApplication>

#include "file_handler.h"
#include "manager.h"

int main(int argc,  char** argv) {
    FileHandler* fh;
    QApplication app (argc, argv);
    Botan::AutoSeeded_RNG rng;
    srand(rng.random_vec(1)[0]);
    std::string choice, path;
    if (argc == 1)
        while (1) {
            choice = input("Would you like to create a new database? ");
            if (choice == "yes") {
                path = fh->newLoc();
                create(path);
                break;
            } else if (choice == "no") {
                path = fh->getDb();
                if (!open(path)) {
                    std::cout << "Invalid database file." << std::endl;
                    continue;
                }
                break;
            }
        }
    else if (!open(argv[1])) return 1;
    std::cout << "You may find the source code at https://github.com/binex-dsk/passmanpp." << std::endl << "Type help for available commands." << std::endl;
    while (1) {
        choice = input("passman> ");

        if (choice == "help")
            std::cout << "add: add a password" << std::endl << "delete: delete a password" << std::endl << "edit: edit an entry" << std::endl << "view: view a password" << std::endl << "tips: tips for password management" << std::endl << "info: some info on passman++" << std::endl << "backup: backup your database" << std::endl << "save: save the database" << std::endl << "exit: exit out of the program" << std::endl;
        else if (choice == "add")
            add();
        else if (choice == "view")
            view();
        else if (choice == "delete")
            del();
        else if (choice == "tips")
            std::cout << "A good password contains:" << std::endl << "- At least 8 characters, but more is always preferred" << std::endl << "- No easy-to-guess phrases or common passwords (See https://github.com/danielmiessler/SecLists/tree/master/Passwords/Common-Credentials)" << std::endl << "- At least one each of a lowercase and uppercase letter, a number, and a special character, but more is always preferred" << std::endl << "- A sequence of characters with no observable pattern (example: things like a1b2c3d4 are generally not preferrable to something like d.Y2/90a)" << std::endl << "- Some sort of meaning that you can use to remember it" << std::endl << "A bad password contains:" << std::endl << "- Less than 8 characters" << std::endl << "- Common, easy-to-guess phrases" << std::endl << "- Sequences of repeated characters or obvious patterns" << std::endl << "- Little variety in the characters" << std::endl << "Never, ever share your passwords. Ever. They are the single most important piece of security in almost everything online. A single person getting your password can cause it to be shared all over the internet, potentially leaking sensitive info." << std::endl << "If you can't think of a good password, hundreds of tools online can help you with that, including on here." << std::endl << "Change your password often. Leaks occur often. Remember to occasionally check https://haveibeenpwned.com/ and enter your email to see if your password may have been leaked." << std::endl << "Do not use the same password for everything. At the very most, use a password on 2 different sites. If someone gets your password and you use the same one for everything, then your accounts will likely be compromised and sensitive info could be leaked." << std::endl << "Do not store your passwords in an easy-to-find location. Either use a password manager like this one, or store it in a place nobody can find. Never just try to \"remember\" them, either--your memory is volatile. A password manager is far better at remembering things than you ever will be." << std::endl << "Lock your computer and phone when not using them, especially if you store passwords on it." << std::endl;
        else if (choice == "info")
            std::cout << "passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help me learn C++." << std::endl << "Project repository: https://github.com/binex-dsk/passmanpp" << std::endl << "FOSS Libraries used: Botan, Qt, libsodium" << std::endl << "Current version: 1.1.0" << std::endl << "Personal message: Thanks so much for using passman++. This has been possibly the single largest learning experience for me, ever. I've put weeks of work into just the initial release, and even more on the current version. All support is greatly appreciated." << std::endl;
        else if (choice == "edit")
            edit();
        else if (choice == "save")
            save(path);
        else if (choice == "backup") {
            FileHandler* fh;
            int br = fh->backup(path);
            if (br == 3)
                std::cout << "Invalid backup location." << std::endl;
            else if (br == 17)
                std::cout << "Improper permissions for file. Please select a location where the current user has write permissions." << std::endl;
            else
                std::cout << "Database backed up successfully." << std::endl;
        }
        else if (choice == "exit")
            if (modified)
                std::cout << "Please save your work before leaving." << std::endl;
            else {
                std::cout << "Thanks for using passman++." << std::endl;
                return 0;
            }
    }
    return app.exec();
}
