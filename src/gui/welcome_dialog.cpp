#include <QPushButton>
#include <QLabel>

#include "welcome_dialog.hpp"
#include "constants.hpp"

void createDatabase(std::shared_ptr<Database> t_database, QString t_path) {
    if (t_path.isEmpty()) {
        t_path = newLoc();
        if (t_path.isEmpty()) {
            std::exit(1);
        }
    }

    t_database->path = t_path;

    if (!t_database->config(true)) {
        std::exit(1);
    }
}

WelcomeDialog::WelcomeDialog(std::shared_ptr<Database> t_database)
    : database(t_database)
{
    layout = new QGridLayout(this);

    btnCreate = new QPushButton(tr("Create new database"));
    btnOpen = new QPushButton(tr("Open existing database"));

    label = new QLabel(tr(("Welcome to passman++ " + Constants::passmanVersion).data()));
}

void WelcomeDialog::setup() {
    layout->setContentsMargins(0, 0, 0, 0);

    resize(600, 300);

    QObject::connect(btnCreate, &QPushButton::clicked, [this]() mutable {
        accept();
        createDatabase(this->database);
    });

    QObject::connect(btnOpen, &QPushButton::clicked, [this]() mutable {
        accept();
        openDb(QFileDialog::getOpenFileName(nullptr, tr("Open Database"), "", Constants::fileExt));
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
