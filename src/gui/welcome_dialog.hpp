#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include <QDialog>
#include <QGridLayout>
#include <QLabel>

#include "database.hpp"

class WelcomeDialog : public QDialog
{
public:
    WelcomeDialog(std::shared_ptr<Database> t_database);

    void setup();
    int show();

    void create(QString path = "");
    void openDb(const QString &path);

    std::shared_ptr<Database> database;

    QGridLayout *layout;

    QPushButton *btnCreate;
    QPushButton *btnOpen;

    QLabel *label;
};

#endif // WELCOMEDIALOG_H
