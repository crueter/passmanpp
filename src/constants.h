#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "util/extra.h"

static const int MAX_SUPPORTED_VERSION_NUMBER = 6;
static const QList<std::string> checksumMatch = {"Blake2b", "SHA-3", "SHAKE-256", "Skein-512", "SHA-512"};
static const QList<std::string> hashMatch = {"Argon2id", "Bcrypt-PBKDF", "No hashing, only derivation"};
static const QList<std::string> encryptionMatch = {"AES-256/GCM", "Twofish/GCM", "SHACAL2/EAX", "Serpent/GCM"};

static const std::string PASSMAN_VERSION = "2.0.0rc2";
static const std::string BOTAN_VERSION = std::to_string(BOTAN_VERSION_MAJOR) + "." + std::to_string(BOTAN_VERSION_MINOR) + "-" + std::to_string(BOTAN_VERSION_PATCH);

static const std::string tips = "A good password contains:\n"
                          "- At least 8 characters, but prefer 15 or more\n"
                          "- No easy-to-guess phrases or common passwords (See https://github.com/danielmiessler/SecLists/tree/master/Passwords/Common-Credentials)\n"
                          "- At least one each of a lowercase and uppercase letter, a number, and a special character\n"
                          "- A sequence of characters with no observable pattern (example: things like a1b2c3d4 shouldn't be used over something like d.Y2/90a)\n"
                          "- Some sort of meaning that you can use to remember it (i.e mbodHis12! = \"my big old dog Harris is 12!\")\n\n"
                          "A bad password contains:\n"
                          "- Less than 8 characters\n"
                          "- Common, easy-to-guess phrases\n"
                          "- Sequences of repeated characters or obvious patterns\n"
                          "- Little variety in the characters\n\n"
                          "Never, ever share your passwords. Ever. They are the single most important piece of security in almost everything online. A single person getting your password can cause it to be shared all over the internet, potentially leaking sensitive info.\n\n"
                          "If you can't think of a good password, hundreds of tools online can help you with that, or you can use the random password generator on here.\n\n"
                          "Change your password often. Leaks occur often. Remember to occasionally check https://haveibeenpwned.com/ and enter your email to see if your password may have been leaked.\n"
                          "Do not use the same password for everything. At the very most, use a password on 2 different sites. If someone gets your password and you use the same one for everything, then your accounts will likely be compromised and sensitive info could be leaked.\n\n"
                          "Do not store your passwords in an easy-to-find location. Either use a password manager like this one, or store it in a place nobody can find. Never just try to \"remember\" them, either--your memory is volatile. A password manager is far better at remembering things than you ever will be.\n\n"
                          "No matter how securely a website stores its passwords, the strength of your password is always the weakest link. Check with online password strength checkers to verify your password is strong enough.";

static const std::string github = "https://github.com/binex-dsk/passmanpp";
static const std::string info = "passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help me learn C++.\n"
                          "Project repository: " + github + "\n"
                          "FOSS Libraries used: Botan, Qt, libsodium\n"
                          "Botan version: " + BOTAN_VERSION + "\n"
                          "Current version: " + PASSMAN_VERSION;

static const std::string help = "edit: add, modify, and delete entries\n"
                                "tips: tips for password management\n"
                                "info: some info on passman++\n"
                                "backup: backup your database\n"
                                "save: save the database\n"
                                "config: change database configuration\n"
                                "exit: exit out of the program";

static const std::string usage = "usage: ./passman [PATH] (COMMANDS...)\n"
                                 "where COMMAND is one of:\n" + help
                                 + "\n\n./passman [COMMAND]\n"
                                 "where COMMAND is one of:\n"
                                 "new: create a new database\n"
                                 "help: this help message\n"
                                 "tips: view tips for password management\n"
                                 "info: some info about passman++\n\n"
                                 "submit all bug reports, or take a look at the source code, at " + github + ".";

static const QString reuseWarning = "This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!";
static const QString shortWarning = "Please make your password at least 8 characters. This is the common bare minimum for many websites, and is the shortest password you can have that can't be easily bruteforced.";

static const QString _allF = ";;All Files (*)";
static const QString fileExt = "passman++ Database Files (*.pdpp)" + _allF;
static const QString keyExt = "passman++ Key Files (*.pkpp)" + _allF;
static const QString choosingUrl = "https://github.com/binex-dsk/passmanpp/blob/main/Choosing%20Options.md";
static const QString whiteSpace = " \t\v\r\n";

static const QList<QChar> capital = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z'};
static const QList<QChar> number = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};
static const QList<QChar> symbol = {'!', '#', '$', '%', '&', '(', ')', '*', '+', ',', '-', '.', '/', ':', ';', '<', '=', '>', '?', '@', '[', ']', '^', '_', '`', '{', '|', '}', '~'};

static const QStringList choices = {"help", "info", "tips", "edit", "backup", "save", "config", "exit"};

extern bool debug;
extern bool verbose;

#endif // CONSTANTS_H
