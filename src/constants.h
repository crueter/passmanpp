#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "util/extra.h"

constexpr int MAX_SUPPORTED_VERSION_NUMBER = 7;
static const QList<std::string> checksumMatch = {"Blake2b", "SHA-3", "SHAKE-256", "Skein-512", "SHA-512"};
static const QList<std::string> hashMatch = {"Argon2id", "Bcrypt-PBKDF", "Scrypt", "No hashing, only derivation"};
static const QList<std::string> encryptionMatch = {"AES-256/GCM", "Twofish/GCM", "SHACAL2/EAX", "Serpent/GCM"};

static const std::string PASSMAN_VERSION = "2.0.0rc6";
static const QString PASSMAN_WELCOME = tr("Welcome to passman++ " + PASSMAN_VERSION);
static const std::string BOTAN_VERSION = std::to_string(BOTAN_VERSION_MAJOR) + "." + std::to_string(BOTAN_VERSION_MINOR) + "-" + std::to_string(BOTAN_VERSION_PATCH);

static const std::string github = "https://github.com/binex-dsk/passmanpp/";
static const QString info = tr("passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help me learn C++.\n"
                          "Project repository (Submit bug reports, suggestions, and PRs here): " + github + "\n"
                          "FOSS Libraries used: Botan, Qt, libsodium\n"
                          "Botan version: " + BOTAN_VERSION + "\n"
                          "Current version: " + PASSMAN_VERSION);

static const std::string usage = "usage: ./passman [PATH]\n"
                                 "./passman [COMMAND] (PATH)\n"
                                 "where COMMAND is one of:\n"
                                 "new: create a new database at PATH, or prompt for the location\n"
                                 "help: this help message\n"
                                 "info: some info about passman++\n\n"
                                 "submit all bug reports, or take a look at the source code, at " + github + ".";

static const QString reuseWarning = "This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!";
static const QString shortWarning = "Please make your password at least 8 characters. This is the common bare minimum for many websites, and is the shortest password you can have that can't be easily bruteforced.";

static const QString _allF = ";;All Files (*)";
static const QString fileExt = "passman++ Database Files (*.pdpp)" + _allF;
static const QString keyExt = "passman++ Key Files (*.pkpp)" + _allF;
static const QString whiteSpace = " \t\v\r\n";

extern bool debug;
extern bool verbose;

#endif // CONSTANTS_H
