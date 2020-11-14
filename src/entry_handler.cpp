#include <QInputDialog>
#include <QTranslator>
#include <QMessageBox>
#include <QCheckBox>
#include <QLabel>
#include <QToolButton>
#include <QFormLayout>
#include <QTextEdit>
#include <QMenu>

#include "entry_handler.h"

void displayErr(std::string msg) {
    QMessageBox err;
    err.setText(QTranslator::tr(msg.c_str()));
    err.setStandardButtons(QMessageBox::Ok);
    err.exec();
}

int EntryHandler::entryInteract() {
    QDialog *dialog = new QDialog();
    QLayout *layout = new QGridLayout(dialog);
    QListWidget *list = new QListWidget(dialog);

    layout->addWidget(list);

    dialog->setLayout(layout);
    list->setWindowTitle(tr("Select an entry"));
    list->setContextMenuPolicy(Qt::CustomContextMenu);
    for (std::string name : getNames())
        list->addItem(QString::fromStdString(name));

    connect(list, &QListWidget::itemDoubleClicked, this, &EntryHandler::dispData);
    connect(list, &QListWidget::customContextMenuRequested, [=](const QPoint& pos) {
        QListWidgetItem *item = list->itemAt(pos);
        if (item == nullptr) return;
        QMenu *rcMenu = new QMenu(list);

        auto delFunc = [=]{
            bool deleted = deleteEntry(item);
            if (deleted) list->takeItem(list->currentRow());
        };

        QAction *delAction = rcMenu->addAction(tr("&Delete..."), delFunc, QKeySequence::Delete);
        delAction->setIcon(QIcon::fromTheme(tr("edit-delete")));

        auto editFunc = [=]{
            editEntry(item);
        };
        QKeySequence editShortcut = QKeySequence(tr("Ctrl+E"));

        QAction *editAction = rcMenu->addAction(tr("&Edit..."), editFunc, editShortcut);
        editAction->setIcon(QIcon::fromTheme(tr("document-edit")));

        auto viewFunc = [=]{
            displayData(item);
        };
        QKeySequence viewShortcut = QKeySequence(tr("Ctrl+V"));

        QAction *viewAction = rcMenu->addAction(tr("&View..."), viewFunc, viewShortcut);
        viewAction->setIcon(QIcon::fromTheme(tr("view-visible")));

        QPoint pos2 = QPoint(dialog->x() + pos.x(), dialog->y() + pos.y());

        rcMenu->exec(pos2);
    });
    dialog->exec();

    return true;
}

void EntryHandler::entryDetails(QString& name, QString& url, QString& email, QString& password, QString& notes) {
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
    connect(random, &QToolButton::clicked, [=]{ passEdit->setText(randomPass()); });

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
    formLayout->addWidget(random);
    formLayout->addWidget(view);

    formLayout->addRow(tr("&Notes:"), notesEdit);

    QToolButton *exit = new QToolButton();
    exit->setText(tr("OK"));
    connect(exit, &QToolButton::clicked, opt, &QDialog::accept);

    formLayout->addWidget(exit);

    opt->setLayout(formLayout);
    opt->exec();

    name = nameEdit->text();
    url = urlEdit->text();
    email = emailEdit->text();
    password = passEdit->text();
    notes = notesEdit->toPlainText();
}

QString EntryHandler::randomPass() {
    QDialog *opt = new QDialog(nullptr);
    opt->setSizePolicy(QSizePolicy());

    opt->setWindowTitle(tr("Random Password Options"));

    QGridLayout *layout = new QGridLayout();

    QLabel *lengthLabel = new QLabel(tr("Length:"));
    QLineEdit *length = new QLineEdit();
    length->setInputMask("0000");

    QCheckBox *capitals = new QCheckBox(tr("Capital Letters"));
    capitals->setCheckState(Qt::CheckState::Checked);

    QCheckBox *numbers = new QCheckBox(tr("Numbers"));
    numbers->setCheckState(Qt::CheckState::Checked);

    QCheckBox *symbols = new QCheckBox(tr("Symbols"));
    symbols->setCheckState(Qt::CheckState::Checked);

    QToolButton *exit = new QToolButton();
    exit->setText(tr("OK"));
    connect(exit, SIGNAL(clicked()), opt, SLOT(accept()));

    layout->addWidget(lengthLabel, 0, 0);
    layout->addWidget(length, 0, 1);
    layout->addWidget(capitals, 2, 0, 2, 1);
    layout->addWidget(numbers, 4, 0, 2, 1);
    layout->addWidget(symbols, 6, 0, 2, 1);
    layout->addWidget(exit, 8, 1, 1, 2, Qt::AlignRight);

    opt->setLayout(layout);
    opt->exec();

    return QString::fromStdString(genPass(length->text().toInt(), capitals->checkState() != 2, numbers->checkState() != 2, symbols->checkState() != 2));
}

int EntryHandler::addEntry() {
    QString name, url, email, notes, password;
    while(1) {
        entryDetails(name, email, url, password, notes);

        if (name == "")
            displayErr("Entry must have a name.");
        else if (exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\""))
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        else if (exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\""))
            displayErr("This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!");
        else break;
    }

    std::string snotes = "\"" + notes.toStdString() + "\"";

    replaceAll(snotes, "\n", " || char(10) || ");
    replaceAll(snotes, "||  ||", "||");
    int arc = exec("INSERT INTO data (name, email, url, notes, password) VALUES (\"" + name.toStdString() + "\", \"" + email.toStdString() + "\", \"" + url.toStdString() + "\", " + snotes + ", \"" + password.toStdString() + "\")");
    modified = true;
    std::cout << "Entry \"" << name.toStdString() << "\" successfully added." << std::endl;
    return arc;
}

int EntryHandler::displayData(QListWidgetItem *item) {
    return exec("SELECT * FROM data WHERE name=\"" + item->text().toStdString() + "\"", false, showData);
}

void EntryHandler::dispData(QListWidgetItem *item) {
    exec("SELECT * FROM data WHERE name=\"" + item->text().toStdString() + "\"", false, showData);
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

int EntryHandler::_editData(void *list, int count, char **data, char **cols) {
    EntryHandler *eh = new EntryHandler;
    QString name = data[0];
    QString origName = name;
    QString email = data[1];
    QString url = data[2];
    QString password = data[4];

    std::string notes = data[3];
    replaceAll(notes, "char(10)", "\n");
    replaceAll(notes, " || ", "");
    QString qnotes = QString::fromStdString(notes);
    while (1) {
        eh->entryDetails(name, email, url, password, qnotes);

        if (name == "")
            displayErr("Entry must have a name.");
        else if (name != origName && exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\""))
            displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
        if (exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\""))
            displayErr("This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!");
        else break;
    }

    std::string stmt = "UPDATE data SET name = \"" + name.toStdString() + "\", email = \"" + email.toStdString() + "\", url = \"" + url.toStdString() + "\", notes = \"" + qnotes.toStdString() + "\", password = \"" + password.toStdString() + "\" WHERE name = \"" + name.toStdString() + "\"";

    int arc = exec(stmt);
    modified = true;
    std::cout << "Entry \"" << name.toStdString() << "\" successfully edited." << std::endl;
    return arc;
}
