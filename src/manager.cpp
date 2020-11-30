#include "manager.h"
#include "database.h"
#include "entry_handler.h"

bool open(Database db) {
    if (std::experimental::filesystem::exists(db.path)) {
        db.parse();
        try {
            std::string p = db.decrypt(" to login");
            if (p == "") {
                return false;
            }
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        std::string line;
        std::istringstream iss(db.stList);
        while (std::getline(iss, line)) {
            exec(line, db, false);
        }
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

bool save(Database db) {
    try {
        db.save();
    } catch (std::exception& e) {
        displayErr(e.what());
        return false;
    }
    db.modified = false;
    return true;
}
