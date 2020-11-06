#include <QListWidgetItem>

#include "security.h"
#include "manager.h"

bool open(std::string path) {
    if (std::experimental::filesystem::exists(path)) {
        pdb.open(path, std::ios_base::binary | std::ios_base::out | std::ios_base::in | std::ios_base::ate);
        std::string r;
        try {
            r = getmpass(" to login", path)[0];
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        std::string line;
        std::istringstream iss(r);
        while (std::getline(iss, line))
            exec(line);
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

bool create(std::string path) {
    pdb.open(path);

    std::string pw = QInputDialog::getText(nullptr, QTranslator::tr("Create Database"), QTranslator::tr("Welcome! To start, please set a master password: "), QLineEdit::Password).toStdString();
    int arc = exec("CREATE TABLE data (name text, email text, url text, notes text, password text)");
    encrypt(pw, path);
    return arc;
}

bool save(std::string path) {
    if (!modified) std::cout << "The database is already up to date." << std::endl;
    else {
        std::vector<std::string> mp;
        try {
            mp = getmpass(" to save", path);
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        encrypt(mp[1], path);
        modified = false;
    }
    return true;
}
