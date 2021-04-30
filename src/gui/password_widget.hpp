#ifndef PASSWORDWIDGET_H
#define PASSWORDWIDGET_H
#include <QWidget>
#include <QString>
#include <passman/extra.hpp>

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
    PasswordWidget(Database *t_database, const passman::PasswordOptionsFlag t_options);

    bool setup();
    void show();

    passman::PasswordOptionsFlag options;

    QLabel *titleLabel;
    QLabel *pathLabel;

    QWidget *inputWidget;
    QGridLayout *inputLayout;

    QLabel *passLabel;
    QLineEdit *passEdit;

    QLineEdit *keyEdit;
    QLabel *keyLabel;
    QAction *getKf;

    QLabel *errLabel;
};

#endif // PASSWORDWIDGET_H
