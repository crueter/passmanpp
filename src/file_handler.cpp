#include "db.h"

#include <QtWidgets>
#include <iostream>
class FileHandler : public QWidget
{

public:
    FileHandler(QWidget *parent = nullptr);
    virtual ~FileHandler();
    int backup() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Location", "", tr("passman++ Database Files (*.pdpp);;All Files (*)"));
        if (fileName.isEmpty()) return 3;
        QFile file(fileName);
        if (!file.open(QIODevice::WriteOnly)) return 17;
        out.setVersion(QDataStream::Qt_4_5);
        std::vector<std::string> mp = getmpass(" to backup");
        encrypt(mp[1], Botan::secure_vector<uint8_t>(mp[2].begin(), mp[2].end()), fileName);
        return 0;
    }
};
