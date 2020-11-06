#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include <QFileDialog>
#include <QIODevice>

#include "db.h"

class FileHandler : public QWidget
{
    Q_OBJECT
public:
    int backup(std::string path);
    std::string newLoc();
    std::string getDb();
};


#endif // FILE_HANDLER_H
