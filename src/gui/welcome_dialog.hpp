#ifndef WELCOMEDIALOG_H
#define WELCOMEDIALOG_H

#include "../database.hpp"

class QDialog;
class QGridLayout;
class QLabel;

class VectorUnion;

void createDatabase(std::shared_ptr<Database> t_database, QString t_path = "");

class WelcomeDialog : public QDialog
{
public:
    WelcomeDialog(std::shared_ptr<Database> t_database);

    inline void openDb(const QString &path) {
        database->path = path;

        if (!database->open()) {
            std::exit(1);
        }
    }

    void setup();
    int show();


    std::shared_ptr<Database> database;

    QGridLayout *layout;

    QPushButton *btnCreate;
    QPushButton *btnOpen;

    QLabel *label;
};

#endif // WELCOMEDIALOG_H
