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

    QToolButton *ok = new QToolButton();
    ok->setText(tr("OK"));
    connect(ok, &QToolButton::clicked, dialog, &QDialog::accept);

    QMenuBar *bar = new QMenuBar(dialog);

    QAction *addButton = this->addButton(QIcon::fromTheme(tr("list-add")), tr("Add a new entry (Shortcut: Ctrl+N)"), QKeySequence(tr("Ctrl+N")), [list, db, this]{
        addEntry(list, db);
    });
    bar->addAction(addButton);

    QAction *delButton = this->addButton(QIcon::fromTheme(tr("edit-delete")), tr("Delete selected entry (Shortcut: Del)"), QKeySequence::Delete, [list, db, this]{
        bool deleted = deleteEntry(list->currentItem(), db);
        if (deleted) list->takeItem(list->currentRow());
    });
    bar->addAction(delButton);

    QAction *editButton = this->addButton(QIcon::fromTheme(tr("document-edit")), tr("Edit or view data of selected entry (Shortcut: Ctrl+E)"), QKeySequence(tr("Ctrl+E")), [list, db, this]{
        editEntry(list->currentItem(), db);
    });
    bar->addAction(editButton);

    layout->setMenuBar(bar);
    layout->addWidget(list);

    dialog->setLayout(layout);
    layout->addWidget(ok);

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
        if (rand != "")
            passEdit->setText(rand);
    });

    QToolButton *view = new QToolButton;
    view->setCheckable(true);
    view->setIcon(QIcon::fromTheme(tr("view-visible")));
    random->setStatusTip(tr("Toggle password view."));

    connect(view, &QToolButton::clicked, [passEdit](bool checked) {
        QLineEdit::EchoMode echo;
        if (checked) echo = QLineEdit::Normal;
        else echo = QLineEdit::Password;
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

    connect(buttonBox, &QDialogButtonBox::accepted, opt, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, opt, &QDialog::reject);

    formLayout->addWidget(buttonBox);

    opt->setLayout(formLayout);
    int ret = opt->exec();

    if (ret == QDialog::Rejected || (name == nameEdit->text() && url == urlEdit->text() && email == emailEdit->text() && password == passEdit->text() && notes == notesEdit->toPlainText()))
        return false;

    name = nameEdit->text();
    url = urlEdit->text();
    email = emailEdit->text();
    password = passEdit->text();
    notes = notesEdit->toPlainText();
    return true;
}

bool EntryHandler::create(Database db) {
    std::string pw;
    while(1) {
        pw = QInputDialog::getText(nullptr, QWidget::tr("Create Database"), QWidget::tr("Welcome! To start, please set a master password: "), QLineEdit::Password).toStdString();
        if (pw == "") {
            displayErr("Password must be provided.");
            continue;
        }
        if (pw.length() < 8) {
            std::cout << "Warning: your password is less than 8 characters long. Consider making it longer." << std::endl;
        }
    }
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

    QLineEdit *name = new QLineEdit;
    name->setPlaceholderText(tr("Name"));
    name->setMaxLength(255);

    QLineEdit *desc = new QLineEdit;
    desc->setPlaceholderText(tr("Description"));
    desc->setMaxLength(255);

    layout->addRow(tr("Database Name:"), name);
    layout->addRow(tr("Database Description"), desc);

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

    std::string keyFileName;
    bool useKeyFile = false;

    QPushButton *keyFile = new QPushButton(tr("Browse..."));
    QWidget::connect(keyFile, &QPushButton::clicked, [keyFileName, useKeyFile, keyFile]() mutable {
        FileHandler *fh = new FileHandler;
        keyFileName = fh->newKeyFile();
        if (keyFileName != "")
            useKeyFile = true;
        keyFile->setText(QString::fromStdString(keyFileName));
    });

    layout->addRow(tr("Key File:"), keyFile);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(di);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox, &QDialogButtonBox::accepted, di, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, di, &QDialog::reject);

    layout->addWidget(buttonBox);

    di->setLayout(layout);
    di->exec();

    int uuidLen = randombytes_uniform(80);
    Botan::AutoSeeded_RNG rng;
    Botan::secure_vector<uint8_t> uuid = rng.random_vec(uuidLen);

    int arc = exec("CREATE TABLE data (name text, email text, url text, notes text, password text)");

    db.checksum = checksumBox->currentIndex() + 1;
    db.deriv = derivBox->currentIndex() + 1;
    db.hash = hashBox->currentIndex() + 1;
    db.hashIters = hashIterSlider->value();
    db.keyFile = useKeyFile;
    db.encryption = encryptionBox->currentIndex();
    db.uuid = toStr(uuid);
    db.name = name->text().toStdString();
    db.desc = desc->text().toStdString();
    db.stList = "CREATE TABLE data (name text, email text, url text, notes text, password text)";

    if (db.name == "") {
        db.name = "None";
    }

    if (db.desc == "") {
        db.desc = "None";
    }

    db.encrypt(pw);

    return arc;
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

    if (ret == QDialog::Accepted)
        return QString::fromStdString(genPass(length->text().toInt(), capitals->checkState() != 2, numbers->checkState() != 2, symbols->checkState() != 2));
    else
        return "";
}

int EntryHandler::addEntry(QListWidget *list, Database db) {
    QString name, url, email, notes, password;
    while(1) {
        entryDetails(name, email, url, password, notes);

        if (name == "")
            displayErr("Entry must have a name.");
        else if (exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\""))
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        else if (exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\""))
            displayErr(reuseWarning);
        else if (password.length() < 8)
            displayErr(shortWarning);
        else break;
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
QAction *EntryHandler::addButton(QIcon icon, QString statusTip, QKeySequence shortcut, Func func) {
    QAction *action = new QAction(icon, "");
    action->setToolTip(statusTip);
    action->setShortcut(shortcut);
    connect(action, &QAction::triggered, func);
    return action;
}

int EntryHandler::editEntry(QListWidgetItem *item, Database db) {
    int arc = exec("SELECT * FROM data WHERE name=\"" + item->text().toStdString() + "\"", db, true, _editData);
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
    } else
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

    bool edited;
    while (1) {
        edited = eh->entryDetails(name, email, url, password, qnotes);
        if (edited == false) break;

        if (name == "")
            displayErr("Entry must have a name.");
        else if (name != origName && exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\""))
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        if (password != origPass && exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\""))
            displayErr(reuseWarning);
        else if (password.length() < 8)
            displayErr(shortWarning);
        else break;
    }
    if (edited == false) return true;

    std::string stmt = "UPDATE data SET name = \"" + name.toStdString() + "\", email = \"" + email.toStdString() + "\", url = \"" + url.toStdString() + "\", notes = \"" + qnotes.toStdString() + "\", password = \"" + password.toStdString() + "\" WHERE name = \"" + name.toStdString() + "\"";

    int arc = exec(stmt);
    std::cout << "Entry \"" << name.toStdString() << "\" successfully edited." << std::endl;
    return arc;
}
