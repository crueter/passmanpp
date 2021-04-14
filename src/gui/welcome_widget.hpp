#ifndef WelcomeWidget_H
#define WelcomeWidget_H

#include "../database.hpp"
#include "mainwindow.hpp"
#include "base_widget.hpp"

class QLabel;

class VectorUnion;

void createDatabase(Database *t_database, QString t_path = "");

class WelcomeWidget : public BaseWidget
{
public:
    WelcomeWidget(Database *t_database);

    inline void openDb(const QString &path) {
        database->path = path;

        if (!database->open()) {
            return;
        }
    }

    bool setup();
    void show();

    QPushButton *btnCreate;
    QPushButton *btnOpen;

    QLabel *label;
};

#endif // WelcomeWidget_H
