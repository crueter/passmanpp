#include "manager.h"
#include "pdpp_handler.h"

bool open(std::string path) {
    if (std::experimental::filesystem::exists(path)) {
        std::ifstream pdb(path, std::ios_base::binary | std::ios_base::out | std::ios_base::in | std::ios_base::ate);
        std::vector<std::string> r;
        std::cout << "decrypting" << std::endl;
        try {
            r = decryptData(path, " to login");
            std::cout << "decrypted" << std::endl;
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        std::string line;
        std::istringstream iss(r[0]);
        while (std::getline(iss, line))
            exec(line);
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

bool save(std::string path) {
    if (!modified) std::cout << "The database is already up to date." << std::endl;
    else {
        try {
            saveFile(path, " to save");
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        modified = false;
    }
    return true;
}
