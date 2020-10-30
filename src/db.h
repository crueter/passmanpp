#include <botan/auto_rng.h>
#include <botan/cipher_mode.h>
#include <botan/sha2_32.h>
#include <botan/hex.h>
#include <botan/secmem.h>
#include <experimental/filesystem>
#include <vector>

#include "sql.h"

unsigned long getPassLength();
uint32_t randomChar();
bool exists(std::string cmd);

std::string genPass();
std::string getpass(std::string prompt = " to confirm this");
std::string addPass();
std::string getNotes();
std::string input(std::string prompt);

void encrypt(std::string akey, Botan::secure_vector<uint8_t> iv, std::string apath = "");
std::vector<std::string> getmpass(std::string txt = "", std::string path = "");
