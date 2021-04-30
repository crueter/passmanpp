#ifndef UTIL_H
#define UTIL_H

#include <QMessageBox>
#include <QIcon>
#include <QApplication>

inline void displayErr(const QString &msg) {
    QMessageBox err;
    err.setText(QObject::tr(msg.toStdString().data()));
    err.setStandardButtons(QMessageBox::Ok);
    err.setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);

    err.exec();
}

inline QIcon getIcon(QString name) {
    return QIcon(":/" + qApp->property("theme").toString() + "/" + name + ".svg");
}

#endif // UTIL_H
