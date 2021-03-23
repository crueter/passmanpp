#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

#include "welcome_widget.hpp"
#include "../constants.hpp"

void createDatabase(Database *t_database, QString t_path) {
    if (t_path.isEmpty()) {
        t_path = QFileDialog::getSaveFileName(nullptr, tr("New Database Location"), "", Constants::fileExt);
        if (t_path.isEmpty()) {
            std::exit(1);
        }
    }

    t_database->path = t_path;

    t_database->config();
}

WelcomeWidget::WelcomeWidget(Database *t_database)
    : btnCreate(new QPushButton(tr("Create new database")))
    , btnOpen(new QPushButton(tr("Open existing database")))
    , label(new QLabel(tr(("Welcome to passman++ " + Constants::passmanVersion).data())))
{
    database = t_database;
    window = t_database->window;

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Close, this);
    layout = new QGridLayout(this);

    title = tr("Welcome to passman++!");
}

bool WelcomeWidget::setup() {
    layout->setContentsMargins(0, 0, 0, 0);

    resize(600, 300);

    QObject::connect(btnCreate, &QPushButton::clicked, [this]() mutable {
        createDatabase(this->database);
    });

    QObject::connect(btnOpen, &QPushButton::clicked, [this]() mutable {
        openDb(QFileDialog::getOpenFileName(nullptr, tr("Open Database"), "", Constants::fileExt));
    });

    connect(buttonBox, &QDialogButtonBox::rejected, [] {
        std::exit(1);
    });

    QFont font;
    font.setPointSize(16);
    font.setBold(true);

    label->setFont(font);
    layout->addWidget(label, 1, 0);

    layout->addWidget(btnOpen, 2, 0);
    layout->addWidget(btnCreate, 3, 0);

    layout->addWidget(buttonBox, 4, 0);

    label->setLayoutDirection(Qt::LeftToRight);
    label->setAlignment(Qt::AlignHCenter | Qt::AlignTop);

    return true;
}

void WelcomeWidget::show() {
    window->setWidget(this);
}
