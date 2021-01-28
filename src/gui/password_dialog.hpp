#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>

#include "database.hpp"

class PasswordDialog : public QDialog
{
public:
    PasswordDialog(Database *t_database, const bool t_convert, const QString &t_txt);

    bool setup();
    const QString show();

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

    QColor cl;
};

#endif // PASSWORDDIALOG_H
