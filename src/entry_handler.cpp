#include <QInputDialog>
#include <QTranslator>
#include <QMessageBox>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QFormLayout>
#include <QTextEdit>
#include <QMenuBar>
#include <QPushButton>
#include <QDialogButtonBox>

#include "entry_handler.h"

void displayErr(std::string msg) {
    QMessageBox err;
    err.setText(QTranslator::tr(msg.c_str()));
    err.setStandardButtons(QMessageBox::Ok);
    err.exec();
}

int EntryHandler::entryInteract() {
    QDialog *dialog = new QDialog;
    QGridLayout *layout = new QGridLayout(dialog);
    QListWidget *list = new QListWidget(dialog);

    QToolButton *ok = new QToolButton();
    ok->setText(tr("OK"));
    connect(ok, &QToolButton::clicked, dialog, &QDialog::accept);

    QMenuBar *bar = new QMenuBar(dialog);

    QAction *addButton = this->addButton(QIcon::fromTheme(tr("list-add")), tr("Add a new entry (Shortcut: Ctrl+N)"), QKeySequence(tr("Ctrl+N")), [=]{
        addEntry(list);
    });
    bar->addAction(addButton);

    QAction *delButton = this->addButton(QIcon::fromTheme(tr("edit-delete")), tr("Delete selected entry (Shortcut: Del)"), QKeySequence::Delete, [=]{
        bool deleted = deleteEntry(list->currentItem());
        if (deleted) list->takeItem(list->currentRow());
    });
    bar->addAction(delButton);

    QAction *editButton = this->addButton(QIcon::fromTheme(tr("document-edit")), tr("Edit or view data of selected entry (Shortcut: Ctrl+E)"), QKeySequence(tr("Ctrl+E")), [=]{
        editEntry(list->currentItem());
    });
    bar->addAction(editButton);

    layout->setMenuBar(bar);
    layout->addWidget(list);

    dialog->setLayout(layout);
    layout->addWidget(ok);

    list->setWindowTitle(tr("Select an entry"));
    for (std::string name : getNames())
        list->addItem(QString::fromStdString(name));

    connect(list, &QListWidget::itemDoubleClicked, this, [=](QListWidgetItem *item){
        editEntry(item);
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
    connect(random, &QToolButton::clicked, [=]{
        QString rand = randomPass();
        if (rand != "")
            passEdit->setText(rand);
    });

    QToolButton *view = new QToolButton;
    view->setCheckable(true);
    view->setIcon(QIcon::fromTheme(tr("view-visible")));
    random->setStatusTip(tr("Toggle password view."));

    connect(view, &QToolButton::clicked, [=](bool checked) {
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

QString EntryHandler::randomPass() {
    QDialog *opt = new QDialog;
    opt->setSizePolicy(QSizePolicy());

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

int EntryHandler::addEntry(QListWidget *list) {
    QString name, url, email, notes, password;
    while(1) {
        entryDetails(name, email, url, password, notes);

        if (name == "")
            displayErr("Entry must have a name.");
        else if (exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\""))
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        else if (exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\""))
            displayErr("This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!");
        else if (password.length() < 8)
            displayErr("Please make your password at least 8 characters. This is the common bare minimum for many websites, and is the shortest password you can have that can't be easily bruteforced.");
        else break;
    }

    std::string snotes = "\"" + notes.toStdString() + "\"";

    replaceAll(snotes, "\n", " || char(10) || ");
    replaceAll(snotes, "||  ||", "||");
    int arc = exec("INSERT INTO data (name, email, url, notes, password) VALUES (\"" + name.toStdString() + "\", \"" + email.toStdString() + "\", \"" + url.toStdString() + "\", " + snotes + ", \"" + password.toStdString() + "\")");

    modified = true;
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

int EntryHandler::editEntry(QListWidgetItem *item) {
    return exec("SELECT * FROM data WHERE name=\"" + item->text().toStdString() + "\"", false, _editData);
}

bool EntryHandler::deleteEntry(QListWidgetItem *item) {
    QMessageBox delChoice;
    delChoice.setText(tr(std::string("Are you sure you want to delete entry \"" + item->text().toStdString() + "\"? This action is IRREVERSIBLE!").c_str()));
    delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delChoice.setDefaultButton(QMessageBox::No);
    int ret = delChoice.exec();

    if (ret == QMessageBox::Yes) {
        exec("DELETE FROM data WHERE name=\"" + item->text().toStdString() + "\"");
        modified = true;

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
            displayErr("This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!");
        else if (password.length() < 8)
            displayErr("Please make your password at least 8 characters. This is the common bare minimum for many websites, and is the shortest password you can have that can't be easily bruteforced.");
        else break;
    }
    if (edited == false) return true;

    std::string stmt = "UPDATE data SET name = \"" + name.toStdString() + "\", email = \"" + email.toStdString() + "\", url = \"" + url.toStdString() + "\", notes = \"" + qnotes.toStdString() + "\", password = \"" + password.toStdString() + "\" WHERE name = \"" + name.toStdString() + "\"";

    int arc = exec(stmt);
    modified = true;
    std::cout << "Entry \"" << name.toStdString() << "\" successfully edited." << std::endl;
    return arc;
}
