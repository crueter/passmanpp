#include <QMessageBox>
#include <botan/auto_rng.h>

#include "extra.hpp"
#include "constants.hpp"

static const QString keyExt = "passman++ Key Files (*.pkpp)" + allF;

secvec toVec(const std::string &str) {
    return secvec(str.begin(), str.end());
}

secvec toVec(const QString &str) {
    return toVec(str.toStdString());
}

secvec toVec(const char *str, const int length) {
    return secvec(str, str + length);
}

const QString tr(const QString &s) {
    return tr(s.toStdString().data());
}

const QString tr(const std::string &s) {
    return tr(s.data());
}

const QString tr(const char *s) {
    return QObject::tr(s);
}

const QString newKeyFile() {
    return QFileDialog::getSaveFileName(nullptr, tr("New Key File"), "", keyExt);
}

const QString getKeyFile() {
    return QFileDialog::getOpenFileName(nullptr, tr("Open Key File"), "", keyExt);
}

void genKey(const QString &path) {
    Botan::AutoSeeded_RNG rng;
    secvec vec = rng.random_vec(128);

    QFile f(path);
    f.open(QIODevice::ReadWrite);
    QDataStream q(&f);

    for (const uint8_t v : vec) {
        q << v;
    }
    f.close();
}

const QString newLoc() {
    return QFileDialog::getSaveFileName(nullptr, tr("New Database Location"), "", fileExt);
}

const QString getDb() {
    return QFileDialog::getOpenFileName(nullptr, tr("Open Database"), "", fileExt);
}

void displayErr(const QString &msg) {
    QMessageBox err;
    err.setText(tr(msg.toStdString().data()));
    err.setStandardButtons(QMessageBox::Ok);
    err.exec();
}
