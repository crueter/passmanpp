#ifndef MANAGER_H
#define MANAGER_H
#include "../handlers/file_handler.h"
#include "../handlers/entry_handler.h"

bool choiceHandle(std::string choice, EntryHandler *eh, FileHandler *fh, Database db);

#endif // MANAGER_H
