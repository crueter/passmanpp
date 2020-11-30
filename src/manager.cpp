#include "manager.h"

bool choiceHandle(std::string choice, EntryHandler *eh, FileHandler *fh, Database db, std::string path) {
    if (choice == "help") {
        std::cout << help << std::endl;
    } else if (choice == "edit") {
        eh->entryInteract(db);
    } else if (choice == "tips") {
        std::cout << tips << std::endl;
    } else if (choice == "info") {
        std::cout << info << std::endl;
    } else if (choice == "save") {
        if (!db.save()) {
            std::cerr << "Cancelled." << std::endl;
        }
    } else if (choice == "backup") {
        int br = fh->backup(db, path);
        if (br == 3) {
            displayErr("Invalid backup location.");
        } else if (br == 17) {
            displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
        } else {
            std::cout << "Database backed up successfully." << std::endl;
        }
    } else if (choice == "exit") {
        if (db.modified) {
            std::cout << "Please save your work before leaving." << std::endl;
        } else {
            exit(0);
        }
    } else {
        std::cerr << "Invalid choice. Type help for available commands." << std::endl;
    }
    return true;
}
