#include <QFileDialog>
#include <QIODevice>

#include "file_handler.h"
#include "entry_handler.h"

int FileHandler::backup(Database db, std::string path) {
    QString fileName = QFileDialog::getSaveFileName(this, tr("Backup Location"), "", tr(fileExt));
    if (fileName.isEmpty()) {
        return 3;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return 17;
    }
    try {
        db.path = path;
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
    QString fileName = QFileDialog::getSaveFileName(this, tr("New Key File"), "", tr("passman++ Key Files (*.pkpp);;All Files (*)"));
    return fileName.toStdString();
}

std::string FileHandler::getKeyFile() {
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open Key File"), "", tr("passman++ Key Files (*.pkpp);;All Files (*)"));
    return fileName.toStdString();
}

bool FileHandler::open(Database db) {
    if (std::experimental::filesystem::exists(db.path)) {
        db.parse();
        try {
            std::string p = db.decrypt(" to login");
            if (p == "") {
                return false;
            }
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        std::string line;
        std::istringstream iss(db.stList);
        while (std::getline(iss, line)) {
            exec(line, db, false);
        }
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}
