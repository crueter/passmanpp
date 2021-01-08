#include <QLineEdit>
#include <QPushButton>

#include "password_dialog.h"

PasswordDialog::PasswordDialog(Database *database, bool convert, QString txt)
{
    this->database = database;
    this->convert = convert;
    this->txt = txt;
}

void PasswordDialog::init() {
    layout = new QGridLayout(this);

    passLabel = new QLabel("Please enter your master password" + txt + ".");
    passEdit = new QLineEdit;

    keyEdit = new QLineEdit;
    keyLabel = new QLabel(tr("Key File:"));
    getKf = new QPushButton(tr("Open"));
    keyBox = new QDialogButtonBox;

    passButtons = new QDialogButtonBox(QDialogButtonBox::Ok);
    errLabel = new QLabel;

    _cl = QColor(218, 68, 83);

    palette.setColor(QPalette::Light, _cl);
    palette.setColor(QPalette::Dark, _cl);
    palette.setColor(QPalette::Window, QColor(218, 68, 83, 196));
    palette.setColor(QPalette::Text, Qt::white);
}

bool PasswordDialog::setup() {
    if (convert) {
        QFile f(database->path);
        f.open(QIODevice::ReadOnly);
        QTextStream pd(&f);
        QString iv = pd.readLine();

        std::vector<uint8_t> ivd;
        try {
            ivd = Botan::hex_decode(iv.toStdString());
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
            database->keyFile = database->keyFilePath != "";
            keyEdit->setText(database->keyFilePath);
        });

        keyBox->addButton(getKf, QDialogButtonBox::ActionRole);
    } else {
        delete keyEdit;
        delete keyLabel;
        delete getKf;
        delete keyBox;
    }

    errLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);
    errLabel->setLineWidth(2);

    errLabel->setMargin(5);

    QObject::connect(passButtons->button(QDialogButtonBox::Ok), &QPushButton::clicked, [this]() mutable {
        QString pw = passEdit->text();

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

QString PasswordDialog::show() {
    int ret = exec();

    if (ret == QDialog::Rejected) {
        return "";
    }

    if (database->keyFile) {
        database->keyFilePath = keyEdit->text();
    }

    return passEdit->text();
}
