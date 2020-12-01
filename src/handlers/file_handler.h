#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include "../util/generators.h"

class FileHandler : public QWidget
{
    Q_OBJECT
public:
    int backup(Database db);
    std::string newLoc();
    std::string getDb();
    std::string newKeyFile();
    std::string getKeyFile();

    bool open(Database db);
};


#endif // FILE_HANDLER_H
