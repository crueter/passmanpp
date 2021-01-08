#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>

#include "database.h"

class WelcomeDialog : public QDialog
{
public:
    WelcomeDialog(Database *database);

    void init();
    void setup();
    int show();

    void create(QString path = "");
    void openDb(QString path);

    Database *database;

    QGridLayout *layout;

    QPushButton *btnCreate;
    QPushButton *btnOpen;

    QLabel *label;
};

#endif // WELCOMEDIALOG_H
