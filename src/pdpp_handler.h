#ifndef PDPP_HANDLER_H
#define PDPP_HANDLER_H
#include <botan/compression.h>
#include <botan/pwdhash.h>
#include <botan/hash.h>
#include <botan/cipher_mode.h>
#include <botan/auto_rng.h>
#include <botan/hex.h>
#include <any>

#include "constants.h"
#include "entry_handler.h"

std::vector<std::string> parseFile(std::string path);

void encryptData(std::string path, int checksum, int deriv, int hash, int hashIters, bool keyFile, int encryption, Botan::secure_vector<uint8_t> uuid, std::string name, std::string desc, std::string data, std::string password);

std::vector<std::string> decryptData(std::string path, std::string txt = "");

#endif // PDPP_HANDLER_H
