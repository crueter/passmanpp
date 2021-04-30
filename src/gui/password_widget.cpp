#include <QLineEdit>
#include <QPushButton>
#include <QGridLayout>
#include <QLabel>
#include <QDialogButtonBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QFile>
#include <QFileDialog>
#include <QSqlRecord>

#include "password_widget.hpp"
#include "../database.hpp"

PasswordWidget::PasswordWidget(Database *t_database, const passman::PasswordOptionsFlag t_options)
    : options(t_options)

    , inputWidget(new QWidget)
    , inputLayout(new QGridLayout(inputWidget))

    , passLabel(new QLabel("Master Password:"))
    , passEdit(new QLineEdit)

    , keyEdit(new QLineEdit)
    , keyLabel(new QLabel(tr("Key File:")))
    , getKf(new QAction(getIcon(tr("document-open")), tr("Open")))

    , errLabel(new QLabel)
{
    inputLayout->setSpacing(10);
    database = t_database;
    window = t_database->window;

    buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok, this);
    layout = new QGridLayout(this);

    if (options & passman::Convert) {
        title = tr("Convert Database");
    } else if (options & passman::Open) {
        title = tr("Open Database");
    } else if (options & passman::Lock) {
        title = tr("Unlock Database");
    }
    layout->setAlignment(Qt::AlignCenter);
    layout->setSpacing(12);

    inputLayout->setContentsMargins(40, 10, 10, 10);

    QFont titleFont;
    titleFont.setBold(true);
    titleFont.setPixelSize(24);

    titleLabel = new QLabel(title);
    titleLabel->setFont(titleFont);
    pathLabel = new QLabel(database->path.asQStr());
}

bool PasswordWidget::setup() {
    if (options & passman::Convert) {
        if (!database->isOld()) {
            return false;
        }
    }

    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setCursorPosition(0);

    if (database->keyFile) {
        QObject::connect(getKf, &QAction::triggered, [this]() mutable {
            database->keyFilePath = QFileDialog::getOpenFileName(nullptr, tr("Open Key File"), "", passman::Constants::keyExt);
            database->keyFile = !database->keyFilePath.empty();
            keyEdit->setText(database->keyFilePath.asQStr());
        });

        keyEdit->addAction(getKf, QLineEdit::TrailingPosition);
    }

    errLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    errLabel->setLineWidth(2);

    errLabel->setMargin(5);

    QString waitStyle = tr("color: #808080;");
    QString textStyle = ("color: #eff0f1;");
    QString errStyle = tr("padding: 5px; background-color: rgba(218, 68, 63, 196); color: white;");
    QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [waitStyle, textStyle, errStyle, this]() mutable {
        const QString pw = passEdit->text();

        setCursor(QCursor(Qt::WaitCursor));

        passLabel->setStyleSheet(waitStyle);
        buttonBox->setStyleSheet(waitStyle);

        repaint();

        if (database->keyFile) {
            database->keyFilePath = keyEdit->text();
        }

        if (pw.isEmpty()) {
            return window->back();
        }

        int ok = database->verify(pw);

        if (ok) {
            if (options & passman::Open) {
                if (!(options & passman::Convert)) {
                    for (const QString &line : database->stList.asQStr().split('\n')) {
                        if (line.isEmpty()) {
                            continue;
                        }

                        QSqlQuery q(passman::db);
                        if (!q.exec(line)) {
                           displayErr("Warning: Error during database initialization: " + q.lastError().text());
                        }
                    }
                }
                database->get();
                return database->edit();
            } else {
                return window->back();
            }
        }

        if (ok == 3) {
            errLabel->setText(tr("Key File is invalid."));
        } else {
            errLabel->setText(tr("Password is incorrect.\nIf this problem continues, the database may be corrupt."));
        }

        layout->addWidget(errLabel, 0, 0);

        errLabel->setStyleSheet(errStyle);
        passLabel->setStyleSheet(textStyle);
        buttonBox->setStyleSheet(textStyle);

        unsetCursor();
    });

    inputLayout->setAlignment(Qt::AlignmentFlag::AlignRight);
    layout->addWidget(titleLabel, 1, 0);
    layout->addWidget(pathLabel, 2, 0);
    layout->addWidget(inputWidget, 3, 0);
    inputLayout->addWidget(passLabel, 0, 0);
    inputLayout->addWidget(passEdit, 1, 0);
    if (database->keyFile) {
        inputLayout->addWidget(keyLabel, 2, 0);
        inputLayout->addWidget(keyEdit, 3, 0);
    }
    layout->addWidget(buttonBox, 4, 0);
    return true;
}

void PasswordWidget::show() {
    passEdit->setFocus();
    window->setWidget(this);
}
