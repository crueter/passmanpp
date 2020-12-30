#include <QMessageBox>

#include "extra.h"
#include "constants.h"

secvec toVec(std::string str) {
    return secvec(str.begin(), str.end());
}

secvec toVec(QString str) {
    return toVec(str.toStdString());
}

secvec toVec(char *str, int length) {
    return secvec(str, str + length);
}

QByteArray toQBA(secvec vec) {
    return QByteArray(reinterpret_cast<const char*>(vec.data()), vec.size());
}

QString toStr(secvec vec) {
    return QString(toQBA(vec));
}

std::string toStdStr(secvec vec) {
    return std::string(vec.begin(), vec.end());
}

QString tr(QString s) {
    return tr(s.toStdString().data());
}

QString tr(const char *s) {
    return QObject::tr(s);
}

QString newKeyFile() {
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("New Key File"), "", keyExt);
    return fileName;
}

QString getKeyFile() {
    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open Key File"), "", keyExt);
    return fileName;
}

QString newLoc() {
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("New Database Location"), "", fileExt);
    return fileName;
}

QString getDb() {
    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open Database"), "", fileExt);
    return fileName;
}

void displayErr(QString msg) {
    QMessageBox err;
    err.setText(tr(msg.toStdString().data()));
    err.setStandardButtons(QMessageBox::Ok);
    err.exec();
}
