#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QFormLayout>
#include <QTextEdit>
#include <QMenuBar>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QDebug>
#include <QDesktopServices>

#include "file_handler.h"
#include "entry_handler.h"

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

    dialog->setWindowTitle(tr("Select an entry"));
    for (QSqlQuery q : selectAll()) {
        qDebug() << q.record();
        list->addItem(q.record().field(0).tableName());
    }

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

int EntryHandler::addEntry(QListWidget *list, Database tdb) {
    QString name, url, email, notes, password;
    while(1) {
        bool ok = entryDetails(name, email, url, password, notes);
        if (!ok) {
            return false;
        }

        if (name == "") {
            displayErr("Entry must have a name.");
        } else if (exists("name", name)) {
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        } else if (exists("password", password)) {
            displayErr(reuseWarning);
        } else if (password.length() < 8) {
            displayErr(shortWarning);
        } else {
            break;
        }
    }

    QString snotes = "\"" + notes + "\"";
    snotes.replace("\n", " || char(10) || ");
    snotes.replace("||  ||", "||");

    QString st = getCreate(name, {"name", "email", "url", "notes", "password"}, {QVariant::String, QVariant::String, QVariant::String, QVariant::String, QVariant::String}, {name, email, url, snotes, password});
    bool ok = db.exec(st).isValid();
    saveSt(tdb);

    tdb.modified = true;
    std::cout << "Entry \"" << name.toStdString() << "\" successfully added." << std::endl;
    list->addItem(name);
    list->sortItems();

    return ok;
}

template <typename Func>
QAction *EntryHandler::addButton(QIcon icon, const char *whatsThis, QKeySequence shortcut, Func func) {
    QAction *action = new QAction(icon, "");
    action->setWhatsThis(tr(whatsThis));
    action->setShortcut(shortcut);
    connect(action, &QAction::triggered, func);
    return action;
}

int EntryHandler::editEntry(QListWidgetItem *item, Database tdb) {
    QString st = "SELECT * FROM " + item->text();
    QSqlQuery q(db);
    q.exec(st);
    bool ok = false;

    while (q.next()) {
        QString name = q.record().value(0).toString();
        QString origName = name;
        QString email = q.record().value(1).toString();
        QString url = q.record().value(2).toString();

        QString password = q.record().value(4).toString();
        QString origPass = password;

        QString notes = q.record().value(3).toString();
        notes.replace("char(10)", "\n");
        notes.replace(" || ", "");

        while (1) {
            bool edited = entryDetails(name, email, url, password, notes);
            if (edited == false) {
                return true;
            }

            if (name == "") {
                displayErr("Entry must have a name.");
            } else if (name != origName && exists("name", name)) {
                displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
            } else if (password != origPass && exists("password", password)) {
                displayErr(reuseWarning);
            } else if (password.length() < 8) {
                displayErr(shortWarning);
            } else {
                break;
            }
        }

        QString stm;

        if (name != origName) {
            db.exec("DROP TABLE " + origName);
            stm = getCreate(name, {"name", "email", "url", "notes", "password"}, {QVariant::String, QVariant::String, QVariant::String, QVariant::String, QVariant::String}, {name, email, url, notes, password});
        } else {
            stm = "UPDATE " + name + " SET name = \"" + name + "\", email = \"" + email + "\", url = \"" + url + "\", notes = \"" + notes + "\", password = \"" + password + "\" WHERE name = \"" + name + "\"";
        }

        ok = db.exec(stm).isValid();
        if (ok) {
            std::cout << "Entry \"" << name.toStdString() << "\" successfully edited." << std::endl;
        }
    }
    q.finish();
    saveSt(tdb);
    tdb.modified = true;
    return ok;
}

bool EntryHandler::deleteEntry(QListWidgetItem *item, Database tdb) {
    QMessageBox delChoice;
    delChoice.setText(tr(std::string("Are you sure you want to delete entry \"" + item->text().toStdString() + "\"? This action is IRREVERSIBLE!").c_str()));
    delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delChoice.setDefaultButton(QMessageBox::No);
    int ret = delChoice.exec();

    if (ret == QMessageBox::Yes) {
        db.exec("DROP TABLE " + item->text());
        tdb.modified = true;

        std::cout << "Entry \"" << item->text().toStdString() << "\" successfully deleted." << std::endl;
        return true;
    }
    return false;
}
