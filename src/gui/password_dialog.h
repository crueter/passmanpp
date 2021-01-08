#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

#include "database.h"

class PasswordDialog : public QDialog
{
public:
    PasswordDialog(Database *database, bool convert, QString txt);

    void init();
    bool setup();
    QString show();

    Database *database;
    bool convert;
    QString txt;

    QGridLayout *layout;

    QLabel *passLabel;
    QLineEdit *passEdit;

    QLineEdit *keyEdit;
    QLabel *keyLabel;
    QPushButton *getKf;
    QDialogButtonBox *keyBox;

    QDialogButtonBox *passButtons;
    QLabel *errLabel;

    QPalette palette;

    QColor _cl;
};

#endif // PASSWORDDIALOG_H
