#ifndef WelcomeWidget_H
#define WelcomeWidget_H
#include <passman/extra.hpp>

#include "../database.hpp"
#include "mainwindow.hpp"
#include "base_widget.hpp"
#include "password_widget.hpp"

class QLabel;

class VectorUnion;

void createDatabase(Database *t_database, QString t_path = "");
bool openDb(Database *t_database, const QString &path);

class WelcomeWidget : public BaseWidget
{
public:
    WelcomeWidget(Database *t_database);

    bool setup();
    void show();

    QPushButton *btnCreate;
    QPushButton *btnOpen;

    QLabel *label;
};

#endif // WelcomeWidget_H
