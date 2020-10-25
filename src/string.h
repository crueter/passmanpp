/* This file contains all definitions of includes,
global variables, etc. because every single file
includes this header. */
#include <string>

#include <botan/auto_rng.h>
#include <botan/cipher_mode.h>
#include <botan/sha2_32.h>
#include <botan/hex.h>
#include <botan/secmem.h>
#include <experimental/filesystem>
#include <iostream>
#include <sstream>
#include <fstream>
#include <ios>
#include <iterator>
#include <vector>
#include <sqlite3.h>
#include <termios.h>
#include <unistd.h>

extern std::ofstream pdb;
extern std::string path, stList;
extern termios tty;

extern sqlite3* db;
extern int rc;
extern bool modified;

void setEcho(bool echo);
void replaceAll(std::string& str, const std::string& from, const std::string& to);
std::string trim(const std::string& line);