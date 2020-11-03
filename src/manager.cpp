#include <QListWidgetItem>

#include "entry_handler.h"
#include "db.h"
#include "manager.h"

bool open(std::string path) {
    if (std::experimental::filesystem::exists(path)) {
        pdb.open(path, std::ios_base::binary | std::ios_base::out | std::ios_base::in | std::ios_base::ate);
        std::string r;
        try {
            r = getmpass(" to login", path)[0];
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return false;
        }
        std::string line;
        std::istringstream iss(r);
        while (std::getline(iss, line))
            exec(line);
        return true;
    }
    std::cout << "Please enter a valid path!" << std::endl;
    return false;
}

bool create(std::string path) {
    pdb.open(path);
    Botan::AutoSeeded_RNG rng;
    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create("AES-256/GCM", Botan::ENCRYPTION);
    Botan::secure_vector<uint8_t> iv = rng.random_vec(enc->default_nonce_length());
    std::string pw = getpass("Welcome! To start, please set a master password: ");
    int arc = exec("CREATE TABLE data (name text, email text, url text, notes text, password text)");
    encrypt(pw, iv, path);
    return arc;
}

bool save(std::string path) {
    if (!modified) std::cout << "The database is already up to date." << std::endl;
    else {
        std::vector<std::string> mp;
        try {
            mp = getmpass(" to save", path);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
            return false;
        }
        encrypt(mp[1], Botan::secure_vector<uint8_t>(mp[2].begin(), mp[2].end()), path);
        modified = false;
    }
    return true;
}
