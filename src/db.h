/* This file leads to including all header files,
hence, main.cpp only includes this file. */
#include "sql.h"

unsigned long getPassLength();
std::string genPass();
bool exists(sqlite3* db, std::string cmd);
std::string getpass(std::string prompt = " to confirm this");
std::string addPass();
std::string getNotes();

void encrypt(std::string akey, Botan::secure_vector<uint8_t> iv);
std::vector<std::string> getmpass(std::string txt = "");