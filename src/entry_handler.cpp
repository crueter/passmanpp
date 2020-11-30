#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QFormLayout>
#include <QTextEdit>
#include <QMenuBar>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QComboBox>
#include <QStringList>

#include "file_handler.h"
#include "entry_handler.h"
#include "constants.h"
#include "stringutil.h"

void displayErr(std::string msg) {
    QMessageBox err;
    err.setText(QWidget::tr(msg.c_str()));
    err.setStandardButtons(QMessageBox::Ok);
    err.exec();
}

int EntryHandler::entryInteract(Database db) {
    QDialog *dialog = new QDialog;
    QGridLayout *layout = new QGridLayout(dialog);
    QListWidget *list = new QListWidget(dialog);

    QDialogButtonBox *ok = new QDialogButtonBox(dialog);
    ok->setStandardButtons(QDialogButtonBox::Ok);
    connect(ok->button(QDialogButtonBox::Ok), &QPushButton::clicked, dialog, &QDialog::accept);

    QMenuBar *bar = new QMenuBar;
    QMenu *menu = bar->addMenu(tr("Edit"));

    QAction *addButton = this->addButton(QIcon::fromTheme(tr("list-add")), "Creates a new entry in the database.", QKeySequence(tr("Ctrl+N")), [list, db, this]{
        addEntry(list, db);
    });

    QAction *delButton = this->addButton(QIcon::fromTheme(tr("edit-delete")), "Deletes the currently selected entry.", QKeySequence::Delete, [list, db, this]{
        bool deleted = deleteEntry(list->currentItem(), db);
        if (deleted) {
            list->takeItem(list->currentRow());
        }
    });

    QAction *editButton = this->addButton(QIcon::fromTheme(tr("document-edit")), "Edit or view all the information of the current entry.", QKeySequence(tr("Ctrl+E")), [list, db, this]{
        editEntry(list->currentItem(), db);
    });

    menu->addActions(QList<QAction *>{addButton, delButton, editButton});

    layout->setMenuBar(bar);
    layout->addWidget(list);

    layout->addWidget(ok);
    dialog->setLayout(layout);

    list->setWindowTitle(tr("Select an entry"));
    for (std::string name : getNames(db))
        list->addItem(QString::fromStdString(name));

    connect(list, &QListWidget::itemDoubleClicked, this, [db, this](QListWidgetItem *item){
        editEntry(item, db);
    });

    dialog->exec();

    return true;
}

bool EntryHandler::entryDetails(QString& name, QString& url, QString& email, QString& password, QString& notes) {
    QDialog *opt = new QDialog;

    QLineEdit *nameEdit = new QLineEdit(name);
    QLineEdit *urlEdit = new QLineEdit(url);
    QLineEdit *emailEdit = new QLineEdit(email);
    QLineEdit *passEdit = new QLineEdit(password);
    passEdit->setEchoMode(QLineEdit::Password);
    QTextEdit *notesEdit = new QTextEdit(notes);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("&Name:"), nameEdit);
    formLayout->addRow(tr("&URL:"), urlEdit);
    formLayout->addRow(tr("&Email:"), emailEdit);

    QToolButton *random = new QToolButton;
    random->setIcon(QIcon::fromTheme(tr("roll")));
    random->setStatusTip(tr("Generate a random password."));
    connect(random, &QToolButton::clicked, [passEdit, this]{
        QString rand = randomPass();
        if (rand != "") {
            passEdit->setText(rand);
        }
    });

    QToolButton *view = new QToolButton;
    view->setCheckable(true);
    view->setIcon(QIcon::fromTheme(tr("view-visible")));
    random->setStatusTip(tr("Toggle password view."));

    connect(view, &QToolButton::clicked, [passEdit](bool checked) {
        QLineEdit::EchoMode echo;
        if (checked) {
            echo = QLineEdit::Normal;
        } else {
            echo = QLineEdit::Password;
        }
        passEdit->setEchoMode(echo);
    });

    formLayout->addRow(tr("&Password:"), passEdit);
    QDialogButtonBox *passButtons = new QDialogButtonBox(opt);
    passButtons->addButton(random, QDialogButtonBox::ActionRole);
    passButtons->addButton(view, QDialogButtonBox::ActionRole);
    formLayout->addWidget(passButtons);

    formLayout->addRow(tr("&Notes:"), notesEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(opt);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, opt, &QDialog::accept);
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, opt, &QDialog::reject);

    formLayout->addWidget(buttonBox);

    opt->setLayout(formLayout);
    int ret = opt->exec();

    if (ret == QDialog::Rejected || (name == nameEdit->text() && url == urlEdit->text() && email == emailEdit->text() && password == passEdit->text() && notes == notesEdit->toPlainText())) {
        return false;
    }

    name = nameEdit->text();
    url = urlEdit->text();
    email = emailEdit->text();
    password = passEdit->text();
    notes = notesEdit->toPlainText();
    return true;
}

bool EntryHandler::create(Database db) {
    QDialog *di = new QDialog;
    di->setWindowTitle("Database Options");

    QFormLayout *layout = new QFormLayout;

    auto comboBox = [layout](std::vector<const char *> vec, const char * label) -> QComboBox* {
        QComboBox *box = new QComboBox;
        QStringList list = QStringList(vec.begin(), vec.end());
        box->addItems(list);
        box->setCurrentIndex(0);
        layout->addRow(tr(label), box);
        return box;
    };

    QLineEdit *pass = new QLineEdit;
    pass->setPlaceholderText(tr("Password"));
    pass->setEchoMode(QLineEdit::Password);

    QLineEdit *name = new QLineEdit;
    name->setPlaceholderText(tr("Name"));
    name->setMaxLength(255);

    QLineEdit *desc = new QLineEdit;
    desc->setPlaceholderText(tr("Description"));
    desc->setMaxLength(255);

    layout->addRow(tr("Password:"), pass);
    layout->addRow(tr("Name:"), name);
    layout->addRow(tr("Description"), desc);

    QComboBox *checksumBox = comboBox(checksumMatch, "Checksum Function:");
    QComboBox *derivBox = comboBox(derivMatch, "Key Derivation Function:");
    QComboBox *hashBox = comboBox(hashMatch, "Password Hashing Function:");
    QComboBox *encryptionBox = comboBox(encryptionMatch, "Data Encryption Function:");

    QSlider *hashIterSlider = new QSlider(Qt::Horizontal);
    hashIterSlider->setRange(8, 255);
    hashIterSlider->setValue(8);

    QLabel *hashIterLabel = new QLabel(tr("Password Hashing Iterations: 8"));
    QWidget::connect(hashIterSlider, &QSlider::valueChanged, [hashIterLabel](int value) {
        hashIterLabel->setText(QString::fromStdString(split(hashIterLabel->text().toStdString(), ':')[0] + ": " + std::to_string(value)));
    });

    layout->addRow(hashIterLabel, hashIterSlider);

    QLineEdit *keyEdit = new QLineEdit;

    std::string keyFilePath;

    QPushButton *newKeyFile = new QPushButton(tr("New"));
    QWidget::connect(newKeyFile, &QPushButton::clicked, [keyEdit, keyFilePath, db]() mutable {
        FileHandler *fh = new FileHandler;
        keyFilePath = fh->newKeyFile();
        if (keyFilePath != "") {
            db.keyFile = true;
        }
        keyEdit->setText(QString::fromStdString(keyFilePath));
    });

    QPushButton *getKeyFile = new QPushButton(tr("Open"));
    QWidget::connect(getKeyFile, &QPushButton::clicked, [keyEdit, keyFilePath, db]() mutable {
        FileHandler *fh = new FileHandler;
        keyFilePath = fh->getKeyFile();
        if (keyFilePath != "") {
            db.keyFile = true;
        }
        keyEdit->setText(QString::fromStdString(keyFilePath));
    });

    QDialogButtonBox *keyBox = new QDialogButtonBox;
    keyBox->addButton(newKeyFile, QDialogButtonBox::ActionRole);
    keyBox->addButton(getKeyFile, QDialogButtonBox::ActionRole);

    layout->addRow(tr("Key File:"), keyEdit);
    layout->addWidget(keyBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(di);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    std::string pw;
    connect(buttonBox, &QDialogButtonBox::accepted, [pw, pass, di]() mutable {
        pw = pass->text().toStdString();
        if (pw == "") {
            displayErr("Password must be provided.");
        } else {
            if (pw.length() < 8) {
                std::cerr << "Warning: your password is less than 8 characters long. Consider making it longer." << std::endl;
            }
            di->accept();
        }
    });
    connect(buttonBox, &QDialogButtonBox::rejected, di, &QDialog::reject);

    layout->addWidget(buttonBox);

    di->setLayout(layout);
    int ret = di->exec();

    if (ret == QDialog::Rejected) {
        return false;
    }

    db.stList = "CREATE TABLE data (name text, email text, url text, notes text, password text)";
    exec(db.stList);

    db.checksum = checksumBox->currentIndex() + 1;
    db.deriv = derivBox->currentIndex() + 1;
    db.hash = hashBox->currentIndex() + 1;
    db.hashIters = hashIterSlider->value();
    db.encryption = encryptionBox->currentIndex() + 1;
    db.name = name->text().toStdString();
    db.desc = desc->text().toStdString();
    pw = pass->text().toStdString();
    db.keyFilePath = keyEdit->text().toStdString();

    if (!std::experimental::filesystem::exists(db.keyFilePath)) {
        genKey(db.keyFilePath);
    }

    if (db.name == "") {
        db.name = "None";
    }
    db.nameLen = db.name.length();

    if (db.desc == "") {
        db.desc = "None";
    }
    db.descLen = db.desc.length();

    db.encrypt(pw);

    return true;
}


QString EntryHandler::randomPass() {
    QDialog *opt = new QDialog;

    opt->setWindowTitle(tr("Random Password Options"));

    QGridLayout *layout = new QGridLayout;

    QLabel *lengthLabel = new QLabel(tr("Length:"));

    QLineEdit *length = new QLineEdit(opt);
    length->setInputMask("0000");
    length->setCursorPosition(0);

    QCheckBox *capitals = new QCheckBox(tr("Capital Letters"));
    capitals->setCheckState(Qt::CheckState::Checked);

    QCheckBox *numbers = new QCheckBox(tr("Numbers"));
    numbers->setCheckState(Qt::CheckState::Checked);

    QCheckBox *symbols = new QCheckBox(tr("Symbols"));
    symbols->setCheckState(Qt::CheckState::Checked);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(opt);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, opt, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, opt, &QDialog::reject);

    layout->addWidget(lengthLabel, 0, 0);
    layout->addWidget(length, 0, 1);
    layout->addWidget(capitals, 2, 0, 2, 1);
    layout->addWidget(numbers, 4, 0, 2, 1);
    layout->addWidget(symbols, 6, 0, 2, 1);
    layout->addWidget(buttonBox);

    opt->setLayout(layout);
    int ret = opt->exec();

    if (ret == QDialog::Accepted) {
        return QString::fromStdString(genPass(length->text().toInt(), capitals->checkState() != 2, numbers->checkState() != 2, symbols->checkState() != 2));
    } else {
        return "";
    }
}

int EntryHandler::addEntry(QListWidget *list, Database db) {
    QString name, url, email, notes, password;
    while(1) {
        bool ok = entryDetails(name, email, url, password, notes);
        if (!ok) {
            return false;
        }

        if (name == "") {
            displayErr("Entry must have a name.");
        } else if (exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\"")) {
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        } else if (exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\"")) {
            displayErr(reuseWarning);
        } else if (password.length() < 8) {
            displayErr(shortWarning);
        } else {
            break;
        }
    }

    std::string snotes = "\"" + notes.toStdString() + "\"";

    replaceAll(snotes, "\n", " || char(10) || ");
    replaceAll(snotes, "||  ||", "||");
    int arc = exec("INSERT INTO data (name, email, url, notes, password) VALUES (\"" + name.toStdString() + "\", \"" + email.toStdString() + "\", \"" + url.toStdString() + "\", " + snotes + ", \"" + password.toStdString() + "\")", db);
    saveSt(db);

    db.modified = true;
    std::cout << "Entry \"" << name.toStdString() << "\" successfully added." << std::endl;
    list->addItem(name);
    list->sortItems();

    return arc;
}

template <typename Func>
QAction *EntryHandler::addButton(QIcon icon, const char *whatsThis, QKeySequence shortcut, Func func) {
    QAction *action = new QAction(icon, "");
    action->setWhatsThis(tr(whatsThis));
    action->setShortcut(shortcut);
    connect(action, &QAction::triggered, func);
    return action;
}

int EntryHandler::editEntry(QListWidgetItem *item, Database db) {
    int arc = exec("SELECT * FROM data WHERE name=\"" + item->text().toStdString() + "\"", db, true, _editData);
    saveSt(db);
    db.modified = true;
    return arc;
}

bool EntryHandler::deleteEntry(QListWidgetItem *item, Database db) {
    QMessageBox delChoice;
    delChoice.setText(tr(std::string("Are you sure you want to delete entry \"" + item->text().toStdString() + "\"? This action is IRREVERSIBLE!").c_str()));
    delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delChoice.setDefaultButton(QMessageBox::No);
    int ret = delChoice.exec();

    if (ret == QMessageBox::Yes) {
        exec("DELETE FROM data WHERE name=\"" + item->text().toStdString() + "\"", db);
        db.modified = true;

        std::cout << "Entry \"" << item->text().toStdString() << "\" successfully deleted." << std::endl;
        return true;
    }
    return false;
}

int EntryHandler::_editData(void *, int, char **data, char **) {
    EntryHandler *eh = new EntryHandler;
    QString name = data[0];
    QString origName = name;
    QString email = data[1];
    QString url = data[2];

    QString password = data[4];
    QString origPass = password;

    std::string notes = data[3];
    replaceAll(notes, "char(10)", "\n");
    replaceAll(notes, " || ", "");
    QString qnotes = QString::fromStdString(notes);

    while (1) {
        bool edited = eh->entryDetails(name, email, url, password, qnotes);
        if (edited == false) {
            return true;
        }

        if (name == "") {
            displayErr("Entry must have a name.");
        } else if (name != origName && exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\"")) {
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        }
        if (password != origPass && exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\"")) {
            displayErr(reuseWarning);
        } else if (password.length() < 8) {
            displayErr(shortWarning);
        } else {
            break;
        }
    }

    std::string stmt = "UPDATE data SET name = \"" + name.toStdString() + "\", email = \"" + email.toStdString() + "\", url = \"" + url.toStdString() + "\", notes = \"" + qnotes.toStdString() + "\", password = \"" + password.toStdString() + "\" WHERE name = \"" + name.toStdString() + "\"";

    int arc = exec(stmt);
    std::cout << "Entry \"" << name.toStdString() << "\" successfully edited." << std::endl;
    return arc;
}
