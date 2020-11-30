#ifndef MANAGER_H
#define MANAGER_H
#include "constants.h"
#include "file_handler.h"
#include "entry_handler.h"
#include <iostream>

bool choiceHandle(std::string choice, EntryHandler *eh, FileHandler *fh, Database db, std::string path);

#endif // MANAGER_H
