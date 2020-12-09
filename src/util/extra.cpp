#include "extra.h"
#include "constants.h"

Botan::secure_vector<uint8_t> toVec(std::string str) {
    return Botan::secure_vector<uint8_t>(str.begin(), str.end());
}

Botan::secure_vector<uint8_t> toVec(char *str, int length) {
    //return Botan::secure_vector<uint8_t>(str, str + length);
    qDebug() << str;
    return toVec(std::string(str, length));
}

std::string toStr(Botan::secure_vector<uint8_t> vec) {
    return std::string(vec.begin(), vec.end());
}

QString tr(std::string s) {
    return QWidget::tr(s.c_str());
}

QString tr(const char *s) {
    return QWidget::tr(s);
}

std::string newKeyFile() {
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("New Key File"), "", tr(keyExt));
    return fileName.toStdString();
}

std::string getKeyFile() {
    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open Key File"), "", tr(keyExt));
    return fileName.toStdString();
}

std::string newLoc() {
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("New Database Location"), "", tr(fileExt));
    return fileName.toStdString();
}

std::string getDb() {
    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open Database"), "", tr(fileExt));
    return fileName.toStdString();
}
