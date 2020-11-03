#ifndef ENTRY_HANDLER_H
#define ENTRY_HANDLER_H
#include <QListWidgetItem>
#include <QInputDialog>
#include <QLineEdit>
#include <QMessageBox>
#include <QGridLayout>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QToolButton>
#include <QFormLayout>
#include <QTextEdit>
#include <QLayout>

#include "db.h"

class EntryHandler : public QWidget {
    Q_OBJECT
public:
    int entryInteract(const char* slot) {
        QListWidget *list = new QListWidget();
        list->setWindowTitle(tr("Select an entry"));
        for (std::string name : getNames())
            list->addItem(QString::fromStdString(name));

        connect(list, SIGNAL(itemDoubleClicked(QListWidgetItem*)), this, slot);

        QLayout *layout = new QGridLayout();
        QDialog *dialog = new QDialog();

        layout->addWidget(list);

        dialog->setLayout(layout);
        dialog->exec();

        return true;
    }

    void displayErr(std::string msg) {
        QMessageBox err;
        err.setText(tr(msg.c_str()));
        err.setStandardButtons(QMessageBox::Ok);
        err.exec();
    }

    void entryDetails(QString& name, QString& url, QString& email, QString& notes) {
        QDialog *opt = new QDialog(nullptr);

        QLineEdit *nameEdit = new QLineEdit();
        nameEdit->setText(name);

        QLineEdit *urlEdit = new QLineEdit();
        urlEdit->setText(url);

        QLineEdit *emailEdit = new QLineEdit();
        emailEdit->setText(email);

        QTextEdit *notesEdit = new QTextEdit();
        notesEdit->setText(notes);

        QFormLayout *formLayout = new QFormLayout;
        formLayout->addRow(tr("&Name:"), nameEdit);
        formLayout->addRow(tr("&URL:"), urlEdit);
        formLayout->addRow(tr("&Email:"), emailEdit);
        formLayout->addRow(tr("&Notes:"), notesEdit);

        QToolButton *exit = new QToolButton();
        exit->setText(tr("OK"));
        connect(exit, SIGNAL(clicked()), opt, SLOT(accept()));

        formLayout->addWidget(exit);

        opt->setLayout(formLayout);
        opt->exec();

        name = nameEdit->text();
        url = urlEdit->text();
        email = emailEdit->text();
        notes = notesEdit->toPlainText();
    }

    QString randomPass() {
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

    QString addPass() {
        QString password;
        QMessageBox passChoice;
        passChoice.setText("Would you like to input your own password?");
        passChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        passChoice.setDefaultButton(QMessageBox::Yes);

        int ret = passChoice.exec();

        if (ret == QMessageBox::Yes)
            while (1) {
                password = QInputDialog::getText(nullptr, tr("Entry Password"), tr("Password:"), QLineEdit::Password, tr("password"));
                if (exists("SELECT * FROM data WHERE password=\"" + password.toStdString() + "\"")) {
                    displayErr("This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!");
                    continue;
                }
                break;
            }
        else if (ret == QMessageBox::No)
            password = randomPass();
        return password;
    }

    int addEntry() {
        QString name, url, email, notes, password;
        while(1) {
            entryDetails(name, email, url, notes);

            if (name == "")
                displayErr("Entry must have a name.");
            else if (exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\""))
                displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
            else break;
        }

        password = addPass();

        std::string snotes = "\"" + notes.toStdString() + "\"";

        replaceAll(snotes, "\n", " || char(10) || ");
        replaceAll(snotes, "||  ||", "||");
        int arc = exec("INSERT INTO data (name, email, url, notes, password) VALUES (\"" + name.toStdString() + "\", \"" + email.toStdString() + "\", \"" + url.toStdString() + "\", " + snotes + ", \"" + password.toStdString() + "\")");
        modified = true;
        std::cout << "Entry \"" << name.toStdString() << "\" successfully added." << std::endl;
        return arc;
    }

    int displayNames() {
        return entryInteract(SLOT(dispData(QListWidgetItem*)));
    }
    int editEntry() {
        return entryInteract(SLOT(editData(QListWidgetItem*)));
    }
    int deleteEntry() {
        return entryInteract(SLOT(delEntry(QListWidgetItem*)));
    }

    static int _editData(void *list, int count, char **data, char **cols) {
        EntryHandler *eh = new EntryHandler;
        QString name = data[0];
        QString origName = name;
        QString email = data[1];
        QString url = data[2];
        QString password;

        std::string notes = data[3];
        replaceAll(notes, "char(10)", "\n");
        replaceAll(notes, " || ", "");
        QString qnotes = QString::fromStdString(notes);
        while (1) {
            eh->entryDetails(name, email, url, qnotes);

            if (name == "")
                eh->displayErr("Entry must have a name.");
            else if (name != origName && exists("SELECT * FROM data WHERE name=\"" + name.toStdString() + "\""))
                eh->displayErr("An entry named \"" + name.toStdString() + "\" already exists.");
            else break;
        }

        QMessageBox passChoice;
        passChoice.setText("Would you like to edit your password?");
        passChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        int ret = passChoice.exec();

        if (ret == QMessageBox::Yes)
            password = eh->addPass();
        else if (ret == QMessageBox::No)
            password = data[4];

        std::string stmt = "UPDATE data SET name = \"" + name.toStdString() + "\", email = \"" + email.toStdString() + "\", url = \"" + url.toStdString() + "\", notes = \"" + qnotes.toStdString() + "\", password = \"" + password.toStdString() + "\" WHERE name = \"" + name.toStdString() + "\"";

        int arc = exec(stmt);
        modified = true;
        std::cout << "Entry \"" << name.toStdString() << "\" successfully edited." << std::endl;
        return arc;
    }
public slots:
    static void dispData(QListWidgetItem *item) {
        exec("SELECT * FROM data WHERE name=\"" + item->text().toStdString() + "\"", false, showData);
    }
    static void editData(QListWidgetItem *item) {
        exec("SELECT * FROM data WHERE name=\"" + item->text().toStdString() + "\"", false, _editData);
    }

    static void delEntry(QListWidgetItem *item) {
        QMessageBox delChoice;
        delChoice.setText(tr(std::string("Are you sure you want to delete entry \"" + item->text().toStdString() + "\"? This action is IRREVERSIBLE!").c_str()));
        delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        delChoice.setDefaultButton(QMessageBox::No);
        int ret = delChoice.exec();

        if (ret == QMessageBox::Yes) {
            exec("DELETE FROM data WHERE name=\"" + item->text().toStdString() + "\"");
            modified = true;
            std::cout << "Entry \"" << item->text().toStdString() << "\" successfully deleted." << std::endl;
        }
    }
};

#endif // ENTRY_HANDLER_H
