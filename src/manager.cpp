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

bool add() {
    std::string name, email, url, notes, passw;
    while (1) {
        name = input("What name should this entry have? ");
        if (name == "")
            std::cout << "Entry must have a name." << std::endl;
        else if (exists("SELECT * FROM data WHERE name=\"" + name + "\""))
            std::cout << "An entry named \"" << name << "\" already exists." << std::endl;
        else break;
    }
    email = input("What email is associated with this? Blank for none: ");
    url = input("What URL is this for? Blank for none: ");
    std::cout << "Please type in any notes for this. You may use multiple lines. Press enter twice when done: ";
    notes = getNotes();
    if (notes == "") notes = "\"\"";
    passw = addPass();
    int arc = exec("INSERT INTO data (name, email, url, notes, password) VALUES (\"" + name + "\", \"" + email + "\", \"" + url + "\", " + notes + ", \"" + passw + "\")");
    modified = true;
    std::cout << "Entry \"" << name << "\" successfully added." << std::endl;
    return arc;
}

bool del() {
    int arc;
    while(1) {
        std::string name = input("Delete which entry? ");
        if (!exists("SELECT * FROM data WHERE name=\"" + name + "\"")) {
            std::cout << "An entry with the name \"" << name << "\" does not exist." << std::endl;
            continue;
        }
        arc = exec("DELETE FROM data WHERE name=\"" + name + "\"");
        modified = true;
        std::cout << "Entry \"" << name << "\" successfully deleted." << std::endl;
        break;
    }
    return arc;
}

bool view() {
    int arc;
    std::string choice;
    while (1) {
        std::cout << "What entry do you want to view? Type nothing to view available options: ";
        std::getline(std::cin, choice);
        if (choice == "")
            arc = exec("SELECT name FROM data ORDER BY name", false, showNames);
        else {
            if (!exists("SELECT * FROM data WHERE name=\"" + choice + "\"")) {
                std::cout << "An entry with the name \"" << choice << "\" does not exist." << std::endl;
                continue;
            }
            arc = exec("SELECT * FROM data WHERE name=\"" + choice + "\"", false, showData);
        }
        break;
    }
    return arc;
}

bool edit() {
    std::string npassw, notes, url, email, name;
    while(1) {
        name = input("What entry do you want to edit? Blank input to see all possibilities: ");
        if (name == "")
            exec("SELECT name FROM data ORDER BY name", false, showNames);
        else if (!exists("SELECT * FROM data WHERE name=\"" + name + "\""))
            std::cout << "Entry \"" << name << "\" does not exist." << std::endl;
        else break;
    }
    std::cout << "To keep any previous values the same, besides the password, hit enter without typing anything." << std::endl;
    email = input("What should the new email be? ");
    url = input("What should the new URL be? ");
    std::cout << "What should the new notes be? Like before, hit enter twice when done: ";
    notes = getNotes();
    while (1) {
        std::string choice = input("Do you want a new password? ");
        if (choice == "yes") npassw = addPass();
        else if (choice == "no") npassw = "";
        else {
            std::cout << "Please input yes or no." << std::endl;
            continue;
        }
        break;
    }
    std::string stmt = "UPDATE data SET name = \"" + name + "\"";
    if (email != "") stmt += ", email = \"" + email + "\"";
    if (url != "") stmt += ", url = \"" + url + "\"";
    if (npassw != "") stmt += ", password = \"" + npassw + "\"";
    if (notes != "") stmt += ", notes = " + notes;
    stmt += " WHERE name = \"" + name + "\"";
    int arc = exec(stmt);
    modified = true;
    std::cout << "Entry \"" << name << "\" successfully edited." << std::endl;
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
