#ifndef CONSTANTS_H
#define CONSTANTS_H
#include "stringutil.h"

static int MAX_SUPPORTED_VERSION_NUMBER = 5;
static std::vector<const char*> checksumMatch = {"Blake2b(256)", "SHA-3(256)", "SHAKE-256(256)", "Skein-512", "SHA-256"};
static std::vector<const char*> derivMatch = {"PBKDF2"};
static std::vector<const char*> hashMatch = {"Argon2id", "Bcrypt-PBKDF", "No hashing, only derivation"};
static std::vector<const char*> encryptionMatch = {"AES-256/GCM", "Twofish", "Serpent", "SHACAL2"};
#endif // CONSTANTS_H
