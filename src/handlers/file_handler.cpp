#include <QFileDialog>
#include <QIODevice>

#include "file_handler.h"
#include "entry_handler.h"

int FileHandler::backup(Database db) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Location"), "", tr(fileExt));
    if (fileName.isEmpty()) {
        return 3;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return 17;
    }
    try {
        db.path = fileName.toStdString();
        db.save();
    } catch (std::exception& e) {
        displayErr(e.what());
    }
    return true;
}
std::string FileHandler::newLoc() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Database Location"), "", tr(fileExt));
    return fileName.toStdString();
}
std::string FileHandler::getDb() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Database"), "", tr(fileExt));
    return fileName.toStdString();
}

std::string FileHandler::newKeyFile() {
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Key File"), "", tr(keyExt));
    return fileName.toStdString();
}

std::string FileHandler::getKeyFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Key File"), "", tr(keyExt));
    return fileName.toStdString();
}

bool FileHandler::open(Database tdb) {
    if (std::experimental::filesystem::exists(tdb.path)) {
        tdb.parse();
        if (tdb.stList == "") {
            try {
                std::string p = tdb.decrypt(" to login");
                if (p == "") {
                    return false;
                }
            } catch (std::exception& e) {
                displayErr(e.what());
                return false;
            }
        }
        std::string line;
        std::istringstream iss(tdb.stList);
        while (std::getline(iss, line)) {
            db.exec(QString::fromStdString(line));
        }
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}
