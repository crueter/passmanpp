#ifndef PASSWORDDIALOG_H
#define PASSWORDDIALOG_H
#include <QWidget>
#include <QString>

#include "mainwindow.hpp"
#include "base_widget.hpp"

class QGridLayout;
class QLabel;
class QDialogButtonBox;
class QLineEdit;
class QPushButton;

class Database;

class PasswordWidget : public BaseWidget
{
public:
    PasswordWidget(Database *t_database, const PasswordOptionsFlag t_options);

    bool setup();
    void show();

    PasswordOptionsFlag options;

    QLabel *titleLabel;
    QLabel *pathLabel;

    QWidget *inputWidget;
    QGridLayout *inputLayout;

    QLabel *passLabel;
    QLineEdit *passEdit;

    QLineEdit *keyEdit;
    QLabel *keyLabel;
    QPushButton *getKf;
    QDialogButtonBox *keyBox;

    QLabel *errLabel;
};

#endif // PASSWORDDIALOG_H
