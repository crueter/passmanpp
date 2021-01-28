#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "util/extra.hpp"

constexpr int maxVersion = 7;
const QList<std::string> hmacMatch = {"Blake2b", "SHA-3", "SHAKE-256", "Skein-512", "SHA-512"};
const QList<std::string> hashMatch = {"Argon2id", "Bcrypt-PBKDF", "Scrypt", "No hashing, only derivation"};
const QList<std::string> encryptionMatch = {"AES-256/GCM", "Twofish/GCM", "SHACAL2/EAX", "Serpent/GCM"};

const std::string passmanVersion = "2.0.0";
static const QString passmanWelcome = tr("Welcome to passman++ " + passmanVersion);

const std::string github = "https://github.com/binex-dsk/passmanpp/";
static const QString info = tr("passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help me learn C++.\n"
                          "Project repository (Submit bug reports, suggestions, and PRs here): " + github + "\n"
                          "FOSS Libraries used: Botan, Qt, libsodium\n"
                          "Botan version: " + std::to_string(BOTAN_VERSION_MAJOR) + "." + std::to_string(BOTAN_VERSION_MINOR) + "-" + std::to_string(BOTAN_VERSION_PATCH) + "\n"
                          "Current version: " + passmanVersion);

static const std::string usage = "usage: ./passman [PATH]\n"
                                 "./passman [COMMAND] (PATH)\n"
                                 "where COMMAND is one of:\n"
                                 "new: create a new database at PATH, or prompt for the location\n"
                                 "help: this help message\n"
                                 "info: some info about passman++\n\n"
                                 "submit all bug reports, or take a look at the source code, at " + github + ".";

const QString allF = ";;All Files (*)";
static const QString fileExt = "passman++ Database Files (*.pdpp)" + allF;

extern bool debug;
extern bool verbose;

#endif // CONSTANTS_H
