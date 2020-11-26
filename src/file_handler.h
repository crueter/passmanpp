#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include "db.h"
#include "database.h"

class FileHandler : public QWidget
{
    Q_OBJECT
public:
    int backup(Database db, std::string path);
    std::string newLoc();
    std::string getDb();
    std::string newKeyFile();
};


#endif // FILE_HANDLER_H
