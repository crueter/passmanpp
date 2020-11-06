#ifndef MISC_UI_H
#define MISC_UI_H
#include <botan/auto_rng.h>
#include <botan/cipher_mode.h>
#include <botan/sha2_32.h>
#include <botan/hex.h>
#include <botan/secmem.h>
#include <botan/argon2.h>
#include <botan/pwdhash.h>
#include <QTranslator>
#include <QInputDialog>

#include "entry_handler.h"

void encrypt(std::string akey, std::string apath = "");
std::vector<std::string> getmpass(std::string txt = "", std::string path = "");

#endif // MISC_UI_H
