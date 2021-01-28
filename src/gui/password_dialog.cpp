#include <QLineEdit>
#include <QPushButton>

#include "password_dialog.hpp"

PasswordDialog::PasswordDialog(Database *t_database, const bool t_convert, const QString &t_txt)
    : database(t_database)
    , convert(t_convert)
    , txt(t_txt)
{
    layout = new QGridLayout(this);

    passLabel = new QLabel("Please enter your master password" + t_txt + '.');
    passEdit = new QLineEdit;

    keyEdit = new QLineEdit;
    keyLabel = new QLabel(tr("Key File:"));
    getKf = new QPushButton(tr("Open"));
    keyBox = new QDialogButtonBox;

    passButtons = new QDialogButtonBox(QDialogButtonBox::Ok);
    errLabel = new QLabel;

    cl = QColor(218, 68, 83);

    palette.setColor(QPalette::Light, cl);
    palette.setColor(QPalette::Dark, cl);
    palette.setColor(QPalette::Window, QColor(218, 68, 83, 196));
    palette.setColor(QPalette::Text, Qt::white);
}

bool PasswordDialog::setup() {
    if (convert) {
        QFile f(database->path.asQStr());
        f.open(QIODevice::ReadOnly);
        QTextStream pd(&f);
        VectorUnion iv = pd.readLine();

        VectorUnion ivd;
        try {
            ivd = iv.decoded();
        } catch (...) {
            return false;
        }

        if (ivd.size() != 12) {
            return false;
        }
    }

    setWindowTitle(tr("Enter your password"));

    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setCursorPosition(0);

    if (database->keyFile) {
        QObject::connect(getKf, &QPushButton::clicked, [this]() mutable {
            database->keyFilePath = getKeyFile();
            database->keyFile = !database->keyFilePath.empty();
            keyEdit->setText(database->keyFilePath.asQStr());
        });

        keyBox->addButton(getKf, QDialogButtonBox::ActionRole);
    }

    errLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    errLabel->setLineWidth(2);

    errLabel->setMargin(5);

    QObject::connect(passButtons->button(QDialogButtonBox::Ok), &QPushButton::clicked, [this]() mutable {
        const QString pw = passEdit->text();

        setCursor(QCursor(Qt::WaitCursor));

        QPalette textPal;
        textPal.setColor(QPalette::WindowText, Qt::darkGray);
        textPal.setColor(QPalette::ButtonText, Qt::darkGray);

        passLabel->setPalette(textPal);
        passButtons->setPalette(textPal);

        errLabel->setPalette(textPal);

        repaint();

        if (database->keyFile) {
            database->keyFilePath = keyEdit->text();
        }

        if (pw.isEmpty()) {
            return reject();
        }

        int ok = database->verify(pw, convert);
        if (ok == 1) {
            return accept();
        }

        if (ok == 3) {
            errLabel->setText(tr("Key File is invalid."));
        } else {
            errLabel->setText(tr("Password is incorrect.\nIf this problem continues, the database may be corrupt."));
        }

        if (database->keyFile) {
            layout->addWidget(errLabel, 4, 0);
        } else {
            layout->addWidget(errLabel, 2, 0);
        }

        errLabel->setPalette(palette);

        passLabel->setPalette(QPalette());
        passButtons->setPalette(QPalette());

        unsetCursor();
    });

    layout->addWidget(passLabel);
    layout->addWidget(passEdit, 1, 0);
    if (database->keyFile) {
        layout->addWidget(keyLabel, 2, 0);
        layout->addWidget(keyEdit, 3, 0);
        layout->addWidget(keyBox, 3, 1);
        layout->addWidget(passButtons, 5, 0);
    } else {
        layout->addWidget(passButtons, 3, 0);
    }
    return true;
}

const QString PasswordDialog::show() {
    if (exec() == QDialog::Rejected) {
        return "";
    }

    if (database->keyFile) {
        database->keyFilePath = keyEdit->text();
    }

    return passEdit->text();
}
