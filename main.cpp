#include <botan/rng.h>
#include <botan/auto_rng.h>
#include <botan/cipher_mode.h>
#include <botan/sha2_32.h>
#include <botan/hex.h>
#include <botan/secmem.h>
#include <experimental/filesystem>
#include <iostream>
#include <string>
#include <fstream>
#include <ios>
#include <iterator>
#include <curses.h>
#include <sqlite3.h>
#include <termios.h>
#include <unistd.h>

std::ofstream pdb;
std::string path;
std::string stList = "";
termios tty;

sqlite3* db;
int rc = sqlite3_open(":memory:", &db);

bool exists(sqlite3* db, std::string cmd) {
    int ar;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    ar = sqlite3_step(stmt);
    return (ar == 100);
}
static int showNames(void *list, int count, char **data, char **columns) {
    std::cout << data[0] << std::endl;
    return 0;
}
static int showData(void *list, int count, char **data, char **cols) {
    for (int i = 0; i < count; ++i)
        std::cout << cols[i] << ": " << (data[i] ? data[i] : "none") << std::endl;
    return 0;
}
void exec(std::string cmd, bool save = true) {
    if (save) stList += cmd + "\n";
    sqlite3_exec(db, cmd.c_str(), nullptr, nullptr, nullptr);
}

void echoOff()
{
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void echoOn()
{
    tcgetattr(STDIN_FILENO, &tty);
    tty.c_lflag |= ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

std::string getpass(std::string prompt)
{
    std::cout << prompt;
    echoOff();
    std::string buff;
    std::getline(std::cin, buff);
    echoOn();
    std::cout << std::endl;

    return buff;
}

void encrypt(std::string akey, Botan::secure_vector<uint8_t> iv) {
    std::ofstream pdbp(path, std::ios_base::binary);
    Botan::AutoSeeded_RNG rng;
    Botan::SHA_256 sha;
    Botan::secure_vector<uint8_t> pr = sha.process(akey);
    std::string skey(pr.begin(), pr.end());

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create("AES-256/GCM", Botan::ENCRYPTION);

    enc->set_key(pr);

    Botan::secure_vector<uint8_t> pt(stList.data(), stList.data() + stList.length());
    enc->start(iv);
    enc->finish(pt);
    std::string pts(pt.begin(), pt.end());

    std::string towrite = Botan::hex_encode(iv) + "\n" + pts;
    pdbp << towrite;
    pdbp.close();
}

std::vector<std::string> getmpass(std::string txt = "") {
    std::ifstream pdpp(path, std::ios_base::binary);
    std::string ivh, line;
    std::getline(pdpp, ivh);
    std::vector<uint8_t> ivc = Botan::hex_decode(ivh);
    std::string iv(ivc.begin(), ivc.end());
    std::string r(std::istreambuf_iterator<char>{pdpp}, {});
    Botan::secure_vector<uint8_t> rp(r.begin(), r.end());
    
    std::string mpass;
    while(1) {
        mpass = getpass("Please enter your master password" + txt + ": ");
        Botan::SHA_256 sha;
        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create("AES-256/GCM", Botan::DECRYPTION);
        Botan::secure_vector<uint8_t> pkey = sha.process(mpass);

        decr->set_key(pkey);
        decr->start(ivc);
        try {
            decr->finish(rp);
        } catch (...) {
            std::cout << "Wrong password, please try again." << std::endl;
            continue;
        }
        break;
    }
    std::string rpr(rp.begin(), rp.end());
    return {rpr, mpass, iv};
}
int main(int argc,  char** argv) {
    srand(time(0));
    Botan::AutoSeeded_RNG rng;
    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create("AES-256/GCM", Botan::ENCRYPTION);
    if (argc == 1) {
        while (1) {
            std::string choice;
            std::cout << "Please type the full or relative path of a .pdpp file to use, or \"new\" to create a new database: ";
            std::getline(std::cin, choice);
            if (choice == "new") {
                while (1) {
                    std::cout << "What should the name of this database be? Do NOT use / or .pdpp in the name: ";
                    std::getline(std::cin, path);
                    path = std::string(getenv("PWD")) + "/" + path + ".pdpp";
                    if (std::experimental::filesystem::exists(path)) {
                        std::cout << "A database at this location already exists." << std::endl;
                        continue;
                    }
                    break;
                }
                pdb.open(path);
                Botan::secure_vector<uint8_t> iv = rng.random_vec(enc->default_nonce_length());
                std::string pw = getpass("Welcome! To start, please set a master password: ");
                exec("CREATE TABLE data (name text, email text, url text, notes text, password text)");
                encrypt(pw, iv);
                break;
            } else {
                if (std::experimental::filesystem::exists(choice)) {
                    path = choice;
                    pdb.open(choice, std::ios_base::binary | std::ios_base::out | std::ios_base::in | std::ios_base::ate);
                    std::string r = getmpass(" to login")[0];
                    std::string line;
                    std::istringstream iss(r);
                    while (std::getline(iss, line))
                        exec(line);
                    break;
                }
                else
                    std::cout << "Please enter a valid path!" << std::endl;
            }
        }
    } else {
        if (std::experimental::filesystem::exists(argv[1])) {
            path = argv[1];
            pdb.open(argv[1], std::ios_base::binary | std::ios_base::out | std::ios_base::in | std::ios_base::ate);
            std::string r = getmpass(" to login")[0];
            std::string line;
            std::istringstream iss(r);
            while (std::getline(iss, line))
                exec(line);
        }
        else {
            std::cout << "Please enter a valid path!" << std::endl;
            return 1;
        }
    }
    std::cout << "You may find the source code at https://github.com/binex-dsk/passmanpp.\nType help for available commands." << std::endl;
    while (1) {
        std::string choice;
        std::cout << "passman> ";
        std::cin.clear();

        std::getline(std::cin, choice);
        if (choice == "help")
            std::cout << "add: add a password\ndelete: delete a password\nedit: edit an entry\nview: view a password\ntips: tips for password management\ninfo: some info on passman\nexit: exit out of the program" << std::endl;
        else if (choice == "add") {
            std::string name, email, url, notes, line, passw;
            while (1) {
                std::cout << "What name should this entry have? ";
                std::getline(std::cin, name);
                if (name == "") {
                    std::cout << "Entry must have a name." << std::endl;
                    continue;
                }
                if (exists(db, "SELECT * FROM data WHERE name=\"" + name + "\"")) {
                    std::cout << "An entry named \"" << name << "\" already exists." << std::endl;
                    continue;
                }
                break;
            }
            std::cout << "What email is associated with this? Blank for none: ";
            std::getline(std::cin, email);
            std::cout << "What URL is this for? Blank for none: ";
            std::getline(std::cin, url);
            std::cout << "Please type in any notes for this. You may use multiple lines. Press enter twice when done: ";
            while (std::getline(std::cin, line)) {
                if (line.empty()) break;
                notes += "\"" + line + "\" || char(10) || ";
            }
            while (1) {
                std::cout << "Do you want to input your own password? ";
                std::getline(std::cin, choice);
                if (choice == "yes") {
                    while(1) {
                        passw = getpass("Please enter a password. This must be unique and at least 8 characters: ");
                        if (passw.length() < 8) {
                            std::cout << "Your password must be at least 8 characters. Anything less is prone to bruteforcing, and 8 characters is generally the minimum password length for sites." << std::endl;
                            continue;
                        }
                        if (exists(db, "SELECT * FROM data WHERE password=\"" + passw + "\"")) {
                            std::cout << "This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!" << std::endl;
                            continue;
                        }
                        break;
                    }
                    break;
                } else if (choice == "no") {
                    int length = 0;
                    while(1) {
                        std::string slen;
                        std::cout << "How long should this be? Must be at least 8: ";
                        std::stringstream ss;
                        std::getline(std::cin, slen);
                        ss.str(slen);

                        if (ss >> length) {
                            if (length < 8) {
                                std::cout << "Your password must be at least 8 characters. Anything less is prone to bruteforcing, and 8 is generally the minimum password length for sites." << std::endl;
                                continue;
                            }
                            break;
                        }
                        std::cout << "Please use a number." << std::endl;
                    }
                    std::cout << "Okay, generating a random password." << std::endl;
                    std::string charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ1234567890!@#$%^&*()-_=+[{]},<.>/?";
                    std::cout << rand() * charset.size() << std::endl;
                    while (passw.length() < length)
                        passw += charset[rand() % charset.length()];
                    break;
                }
            }
            if (notes.length() > 0)
                notes = notes.substr(0, notes.length() - 16);
            else notes = "\"\"";
            exec("INSERT INTO data (name, email, url, notes, password) VALUES (\"" + name + "\", \"" + email + "\", \"" + url + "\", " + notes + ", \"" + passw + "\")");
            std::vector<std::string> mp = getmpass(" to confirm this");
            encrypt(mp[1], Botan::secure_vector<uint8_t>(mp[2].begin(), mp[2].end()));
            std::cout << "Entry \"" << name << "\" successfully added." << std::endl;
        } else if (choice == "view") {
            while (1) {
                std::cout << "What entry do you want to view? Type nothing to view available options: ";
                std::getline(std::cin, choice);
                if (choice == "")
                    sqlite3_exec(db, "SELECT name FROM data ORDER BY name", showNames, 0, nullptr);
                else {
                    if (!exists(db, "SELECT * FROM data WHERE name=\"" + choice + "\"")) {
                        std::cout << "An entry with the name \"" << choice << "\" does not exist." << std::endl;
                        continue;
                    }
                    sqlite3_exec(db, ("SELECT * FROM data WHERE name=\"" + choice + "\"").c_str(), showData, 0, nullptr);
                } 
                break;
            }
        } else if (choice == "delete") {
            while(1) {
                std::cout << "Delete which entry? ";
                std::getline(std::cin, choice);
                if (!exists(db, "SELECT * FROM data WHERE name=\"" + choice + "\"")) {
                    std::cout << "An entry with the name \"" << choice << "\" does not exist." << std::endl;
                    continue;
                }
                exec("DELETE FROM data WHERE name=\"" + choice + "\"");
                std::vector<std::string> mp = getmpass(" to confirm this");
                encrypt(mp[1], Botan::secure_vector<uint8_t>(mp[2].begin(), mp[2].end()));
                std::cout << "Entry \"" << choice << "\" successfully deleted." << std::endl;
                break;
            }
        } else if (choice == "tips") {
            std::cout << "A good password contains:" << std::endl << "- At least 8 characters, but more is always preferred" << std::endl << "- No easy-to-guess phrases or common passwords (See https://github.com/danielmiessler/SecLists/tree/master/Passwords/Common-Credentials)" << std::endl << "- At least one each of a lowercase and uppercase letter, a number, and a special character, but more is always preferred" << std::endl << "- A sequence of characters with no observable pattern (example: things like a1b2c3d4 are generally not preferrable to something like d.Y2/90a)" << std::endl << "- Some sort of meaning that you can use to remember it" << std::endl << "A bad password contains:" << std::endl << "- Less than 8 characters" << std::endl << "- Common, easy-to-guess phrases" << std::endl << "- Sequences of repeated characters or obvious patterns" << std::endl << "- Little variety in the characters" << std::endl << "Never, ever share your passwords. Ever. They are the single most important piece of security in almost everything online. A single person getting your password can cause it to be shared all over the internet, potentially leaking sensitive info." << std::endl << "If you can\'t think of a good password, hundreds of tools online can help you with that, including on here." << std::endl << "Change your password often. Leaks occur often. Remember to occasionally check https://haveibeenpwned.com/ and enter your email to see if your password may have been leaked." << std::endl << "Do not use the same password for everything. At the very most, use a password on 2 different sites. If someone gets your password and you use the same one for everything, then your accounts will likely be compromised and sensitive info could be leaked." << std::endl << "Do not store your passwords in an easy-to-find location. Either use a password manager like this one, or store it in a place nobody can find. Never just try to \"remember\" them, either--your memory is volatile. A password manager is far better at remembering things than you ever will be." << std::endl << "Lock your computer and phone when not using them, especially if you store passwords on it." << std::endl;
        } else if (choice == "info") {
            std::cout << "passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help learn C++." << std::endl << "Project repository: https://github.com/binex-dsk/passmanpp" << std::endl << "Libraries used: Botan" << std::endl << "Current version: 0.9.0" << std::endl << "Personal message: Thanks so much for using passman++. This has been possibly the single largest learning experience for me, ever. I've put weeks of work into just the initial release. All support is greatly appreciated." << std::endl;
        } else if (choice == "exit") {
            std::cout << "Thanks for using passman++." << std::endl;
            return 0;
        }
    }
}
