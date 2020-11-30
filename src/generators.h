#ifndef DB_H
#define DB_H
#include <experimental/filesystem>
#include <sodium.h>

#include "sql.h"

uint32_t randomChar();

std::string genPass(int length, bool capitals, bool numbers, bool symbols);
Botan::secure_vector<uint8_t> genKey(std::string path);

#endif
