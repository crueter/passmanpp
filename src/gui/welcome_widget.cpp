#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <passman/extra.hpp>

#include "../passman_constants.hpp"
#include "welcome_widget.hpp"

void createDatabase(Database *t_database, QString t_path) {
    if (t_path.isEmpty()) {
        t_path = QFileDialog::getSaveFileName(nullptr, passman::tr("New Database Location"), "", passman::Constants::fileExt);
        if (t_path.isEmpty()) {
            return;
        }
    }

    t_database->path = t_path;

    t_database->config();
}

bool openDb(Database *t_database, const QString &path) {
    t_database->path = path;

    passman::PasswordOptions options = passman::Open;
    int ok = t_database->parse();

    switch (ok) {
        case 2: {
            options = passman::PasswordOptions(passman::Convert | passman::Open);
            break;
        }
        case 3: {
            displayErr(passman::tr("Invalid magic number. Should be PD++."));
            return false;
        }
        case 4: {
            displayErr(passman::tr("Invalid version number."));
            return false;
        }
        case 5: {
            displayErr(passman::tr("Invalid HMAC option."));
            return false;
        }
        case 6: {
            displayErr(passman::tr("Invalid hash option."));
            return false;
        }
        case 7: {
            displayErr(passman::tr("Invalid encryption option."));
            return false;
        }
        default: {
            break;
        }
    }

    PasswordWidget *di = new PasswordWidget(t_database, options);
    if (!di->setup()) {
        return false;
    }

    di->show();
    return true;
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
        openDb(database, QFileDialog::getOpenFileName(nullptr, tr("Open Database"), "", passman::Constants::fileExt));
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
