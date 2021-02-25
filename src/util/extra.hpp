#ifndef EXTRA_H
#define EXTRA_H
#include <string>
#include <vector>
#include <sstream>
#include <iostream>
#include <fstream>
#include <botan/secmem.h>

#include <QSqlDatabase>
#include <QFileDialog>
#include <QMessageBox>
#include <QIcon>
#include <QApplication>

extern QSqlDatabase db;

typedef Botan::secure_vector<uint8_t> secvec;

// Wrapper function to make translation easier.
inline const QString tr(const QString &s) {
    return QObject::tr(s.toStdString().data());
}

// Wrapper function to make translation easier.
inline const QString tr(const std::string &s) {
    return QObject::tr(s.data());
}

// Wrapper function to make translation easier.
inline const QString tr(const char *s) {
    return QObject::tr(s);
}

// Generate a range list of an integer type.
template <typename NumberType = int>
inline QList<NumberType> range(int start, int amount) {
    QList<NumberType> rangeList;
    for (int i = 0; i < amount; ++i) {
        rangeList.emplaceBack(static_cast<NumberType>(start + i));
    }

    return rangeList;
}

inline const QString newLoc() {
    return QFileDialog::getSaveFileName(nullptr, tr("New Database Location"), "", tr("passman++ Database Files (*.pdpp);;All Files (*)"));
}

inline void displayErr(const QString &msg) {
    QMessageBox err;
    err.setText(tr(msg.toStdString().data()));
    err.setStandardButtons(QMessageBox::Ok);
    err.exec();
}

inline QIcon getIcon(QString name) {
    return QIcon(":/" + qApp->property("theme").toString() + "/" + name + ".svg");
}

#endif
