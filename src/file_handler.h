#ifndef FILE_HANDLER_H
#define FILE_HANDLER_H
#include <QFileDialog>
#include <QIODevice>

#include "db.h"

class FileHandler : public QWidget
{
    Q_OBJECT
public:
    FileHandler(QWidget *parent = nullptr);
    int backup(std::string path) {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Location"), "", tr("passman++ Database Files (*.pdpp);;All Files (*)"));
        if (fileName.isEmpty()) return 3;
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) return 17;
        std::vector<std::string> mp;
        try {
            mp = getmpass(" to backup", path);
        } catch (std::exception& e) {
            std::cout << e.what() << std::endl;
        }
        encrypt(mp[1], Botan::secure_vector<uint8_t>(mp[2].begin(), mp[2].end()), fileName.toStdString());
        return 0;
    }
    std::string newLoc() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("New Database Location"), "", tr("passman++ Database Files (*.pdpp);;All Files (*)"));
        return fileName.toStdString();
    }
    std::string getDb() {
        QString fileName = QFileDialog::getOpenFileName(this, tr("Open Database"), "", tr("passman++ Database Files (*.pdpp);;All Files (*)"));
        return fileName.toStdString();
    }
};


#endif // FILE_HANDLER_H
