#include "string.h"

int showNames(void *list, int count, char **data, char **columns);
int showData(void *list, int count, char **data, char **cols);
int _saveSt(void *list, int count, char **data, char **cols);

void saveSt();
void exec(std::string cmd, bool save = true);