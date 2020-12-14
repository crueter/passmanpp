#include <QCheckBox>
#include <QToolButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QTableView>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QItemDelegate>

#include "entry_handler.h"
#include "sql.h"
#include "generators.h"

void redrawTable(QTableWidget *table) {
    int j = 0;
    QList<QSqlQuery> all = selectAll();
    table->setRowCount(all.length());
    for (QSqlQuery q : all) {
        while (q.next()) {
            for (int i = 0; i < q.record().count() - 1; ++i) {
                table->setItem(j, i, new QTableWidgetItem(q.record().field(i).value().toString().replace(" || char(10) || ", "\n")));
            }
        }
        ++j;
    }
}
void displayErr(std::string msg) {
    QMessageBox err;
    err.setText(QWidget::tr(msg.c_str()));
    err.setStandardButtons(QMessageBox::Ok);
    err.exec();
}

int EntryHandler::entryInteract(Database tdb) {
    QDialog *dialog = new QDialog;

    QDialogButtonBox *ok = new QDialogButtonBox(dialog);
    ok->setStandardButtons(QDialogButtonBox::Ok);
    connect(ok->button(QDialogButtonBox::Ok), &QPushButton::clicked, dialog, &QDialog::accept);

    QGridLayout *layout = new QGridLayout(dialog);

    QTableWidget *table = new QTableWidget(dialog);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);

    QStringList labels{"Name", "Email", "URL", "Notes"};
    table->setColumnCount(labels.length());
    table->setHorizontalHeaderLabels(labels);

    redrawTable(table);

    connect(table, &QTableWidget::itemDoubleClicked, [tdb, this](QTableWidgetItem *item){
        editEntry(item, tdb);
    });

    QAction *addButton = this->addButton(QIcon::fromTheme(tr("list-add")), "Creates a new entry in the database.", QKeySequence(tr("Ctrl+N")), [table, tdb, this]{
        addEntry(table, tdb);
    });

    QAction *delButton = this->addButton(QIcon::fromTheme(tr("edit-delete")), "Deletes the currently selected entry.", QKeySequence::Delete, [table, tdb, this]{
        deleteEntry(table->currentItem(), tdb);
    });

    QAction *editButton = this->addButton(QIcon::fromTheme(tr("document-edit")), "Edit or view all the information of the current entry.", QKeySequence(tr("Ctrl+E")), [table, tdb, this]{
        editEntry(table->currentItem(), tdb);
    });

    QMenuBar *bar = new QMenuBar;
    QMenu *menu = bar->addMenu(tr("Edit"));
    menu->addActions(QList<QAction *>{addButton, delButton, editButton});

    layout->addWidget(table);
    layout->setMenuBar(bar);

    layout->addWidget(ok);

    dialog->setLayout(layout);
    dialog->setWindowTitle(tr("Select an entry"));

    dialog->resize(800, 450);
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
    QTextEdit *notesEdit = new QTextEdit;
    notesEdit->setPlainText(notes);

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("&Name:"), nameEdit);
    formLayout->addRow(tr("&Email:"), emailEdit);
    formLayout->addRow(tr("&URL:"), urlEdit);

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

    nameEdit->setFocus(Qt::FocusReason::MouseFocusReason);
    int ret = opt->exec();

    if (ret == QDialog::Rejected || (name == nameEdit->text() && url == urlEdit->text() && email == emailEdit->text() && password == passEdit->text() && notes == notesEdit->toPlainText())) {
        return false;
    }

    name = nameEdit->text();
    url = urlEdit->text();
    email = emailEdit->text();
    password = passEdit->text();
    notes = notesEdit->toPlainText();
    notes.replace("\n", " || char(10) || ");

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

int EntryHandler::addEntry(QTableWidget *table, Database tdb) {
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

    notes.replace("\n", " || char(10) || ");
    //notes.replace("||  ||", "||");

    QString st = getCreate(name, {"name", "email", "url", "notes", "password"}, {QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString)}, {name, email, url, notes, password});

    execAll(st);
    tdb.stList = saveSt();

    tdb.modified = true;
    std::cout << "Entry \"" << name.toStdString() << "\" successfully added." << std::endl;

    redrawTable(table);
    return true;
}

template <typename Func>
QAction *EntryHandler::addButton(QIcon icon, const char *whatsThis, QKeySequence shortcut, Func func) {
    QAction *action = new QAction(icon, "");
    action->setWhatsThis(tr(whatsThis));
    action->setShortcut(shortcut);
    connect(action, &QAction::triggered, func);
    return action;
}

int EntryHandler::editEntry(QTableWidgetItem *item, Database tdb) {
    QString st = "SELECT * FROM '" + item->tableWidget()->item(item->row(), 0)->text() + "'";
    QSqlQuery q(db);
    q.exec(st);
    bool ok = false;
    QString stmt;

    while (q.next()) {
        QString name = q.record().value(0).toString();
        QString origName = name;
        QString email = q.record().value(1).toString();
        QString url = q.record().value(2).toString();

        QString password = q.record().value(4).toString();
        QString origPass = password;

        QString notes = q.record().value(3).toString();
        notes.replace(" || char(10) || ", "\n");

        while (1) {
            bool edited = entryDetails(name, url, email, password, notes);
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

        if (name != origName) {
            stmt = getCreate(name, {"name", "email", "url", "notes", "password"}, {QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString)}, {name, email, url, notes, password});
            stmt += "DROP TABLE '" + origName + "'\n";
            item->setText(name);
        } else {
            stmt = "UPDATE '" + name + "' SET name = '" + name + "', email = '" + email + "', url = '" + url + "', notes = '" + notes + "', password = '" + password + "' WHERE name = '" + name + "'";
        }

        std::cout << "Entry \"" << name.toStdString() << "\" successfully edited." << std::endl;
    }
    q.finish();
    std::cout << stmt.toStdString() << std::endl;
    execAll(stmt);
    tdb.stList = saveSt();
    tdb.modified = true;

    redrawTable(item->tableWidget());
    return ok;
}

bool EntryHandler::deleteEntry(QTableWidgetItem *item, Database tdb) {
    QString name = item->tableWidget()->item(item->row(), 0)->text();
    QMessageBox delChoice;
    delChoice.setText(tr(std::string("Are you sure you want to delete entry \"" + name.toStdString() + "\"? This action is IRREVERSIBLE!").c_str()));
    delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delChoice.setDefaultButton(QMessageBox::No);
    int ret = delChoice.exec();

    if (ret == QMessageBox::Yes) {
        db.exec("DROP TABLE " + name);
        tdb.modified = true;

        std::cout << "Entry \"" << name.toStdString() << "\" successfully deleted." << std::endl;
        return true;
    }
    redrawTable(item->tableWidget());
    return false;
}
