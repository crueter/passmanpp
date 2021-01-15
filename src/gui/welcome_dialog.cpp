#include <QPushButton>

#include "welcome_dialog.h"
#include "constants.h"

void WelcomeDialog::create(QString path) {
    if (path.isEmpty()) {
        path = newLoc();
        if (path.isEmpty()) {
            delete database;
            exit(1);
        }
    }

    database->path = path;

    bool cr = database->config(true);
    if (!cr) {
        delete database;
        exit(1);
    }
}

void WelcomeDialog::openDb(QString path) {
    database->path = path;

    if (!database->open()) {
        exit(1);
    }
}

WelcomeDialog::WelcomeDialog(Database *_database)
    : database(_database)
{
    layout = new QGridLayout(this);

    btnCreate = new QPushButton(tr("Create new database"));
    btnOpen = new QPushButton(tr("Open existing database"));

    label = new QLabel(PASSMAN_WELCOME);
}

void WelcomeDialog::setup() {
    layout->setContentsMargins(0, 0, 0, 0);

    resize(600, 300);

    QObject::connect(btnCreate, &QPushButton::clicked, [this]() mutable {
        accept();
        create();
    });

    QObject::connect(btnOpen, &QPushButton::clicked, [this]() mutable {
        accept();
        openDb(getDb());
    });

    QFont font;
    font.setPointSize(16);
    font.setBold(true);
    label->setFont(font);

    layout->addWidget(btnCreate, 3, 0);
    layout->addWidget(btnOpen, 2, 0);

    label->setLayoutDirection(Qt::LeftToRight);
    label->setAlignment(Qt::AlignHCenter|Qt::AlignTop);

    layout->addWidget(label, 1, 0);
}

int WelcomeDialog::show() {
    return exec();
}
