#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QDesktopServices>
#include <QPushButton>
#include <QHeaderView>
#include <QToolBar>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMenuBar>
#include <QLabel>
#include <QToolButton>
#include <QDialogButtonBox>
#include <QUrl>

#include <passman/constants.hpp>
#include <passman/pdpp_entry.hpp>

#include "database_main_widget.hpp"
#include "password_widget.hpp"
#include "entry_edit_widget.hpp"
#include "password_generator_dialog.hpp"
#include "../database.hpp"
#include "../actions/password_generator_action.hpp"
#include "../passman_constants.hpp"

passman::PDPPEntry *DatabaseWidget::selectedEntry() {
    QTableWidgetItem *item = table->item(table->currentRow(), 0);

    if (item == nullptr) {
        return nullptr;
    }

    return database->entryNamed(item->text());
}

void DatabaseWidget::redrawTable() {
    int j = 0;
    table->setRowCount(static_cast<int>(this->database->entryLength()));

    for (passman::PDPPEntry *e : database->entries()) {
        for (const int i : passman::range(0, static_cast<int>(e->fieldLength()))) {
            table->setItem(j, i, new QTableWidgetItem(e->fieldAt(i)->dataStr()));
        }
        ++j;
    }
}

DatabaseWidget::DatabaseWidget(Database *t_database)
    : table(new QTableWidget(this))

    , prevWidg(new QWidget)
    , preview(new QGridLayout(prevWidg))

    , nameValue(new QLabel(prevWidg))
    , emailValue(new QLabel(prevWidg))
    , urlValue(new QLabel(prevWidg))
    , passValue(new QLabel(prevWidg))
    , nameLabel(new QLabel(tr("Name"), prevWidg))
    , emailLabel(new QLabel(tr("Email"), prevWidg))
    , urlLabel(new QLabel(tr("URL"), prevWidg))
    , passLabel(new QLabel(tr("Password"), prevWidg))
    , passView(new QToolButton(prevWidg))
{
    database = t_database;
    window = t_database->window;

    buttonBox = new QDialogButtonBox(this);
    layout = new QGridLayout(this);

    toolbar = new QToolBar;
    menubar = new QMenuBar;
    title = tr("Select an entry [*]");

    saveButton = this->addButton("Save", "document-save", "Save the database as is. (Ctrl+S)", QKeySequence(tr("Ctrl+S")), [this] {
        database->save();
        window->setWindowModified(false);
    });
    toolbar->addAction(saveButton);

    saveAsButton = this->addButton("Save as...", "document-save-as", "Save the database to a different location. (Ctrl+Shift+S)", QKeySequence(tr("Ctrl+Shift+S")), [this] {
        int br = database->saveAsPrompt();
        if (br == 3) {
            displayErr("Invalid save location.");
        } else if (br == 17) {
            displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
        } else {
            this->setWindowModified(false);
        }
    });
    toolbar->addAction(saveAsButton);

    fileMenu = menubar->addMenu(tr("File"));

    configButton = this->addButton("Edit Database", "settings-configure", "Edit database options. (Ctrl+Shift+E)", QKeySequence(tr("Ctrl+Shift+E")), [this] {
        database->config(false);
        window->setWindowModified(false);
    });

    toolbar->addSeparator();

    fileMenu->addAction(configButton);

    addEButton = this->addButton("New", "list-add", "Creates a new entry in the database. (Ctrl+N)", QKeySequence(tr("Ctrl+N")), [this]{
        database->add();
        window->setWindowModified(database->modified);
    });
    toolbar->addAction(addEButton);

    delButton = this->addButton("Delete", "edit-delete", "Deletes the currently selected entry. (Delete)", QKeySequence::Delete, [this]{
        passman::PDPPEntry *named = selectedEntry();
        if (named == nullptr) {
            return;
        }

        QTableWidget *t_table = database->widget->table;

        if (t_table == nullptr) {
            return;
        }

        const QString name = t_table->currentItem()->text();
        QMessageBox delChoice(QMessageBox::Warning, passman::tr("Delete entry " + name + '?'), passman::tr("Are you sure you want to delete entry \"" + name + "\"? This action is IRREVERSIBLE!"), QMessageBox::Yes | QMessageBox::No);
        delChoice.setDefaultButton(QMessageBox::No);

        if (delChoice.exec() == QMessageBox::Yes) {
            database->removeEntry(named);
        }
        database->widget->redrawTable();

        window->setWindowModified(database->modified);
    });
    toolbar->addAction(delButton);

    editButton = this->addButton("Edit", "document-edit", "Edit or view all the information of the current entry. (Ctrl+E)", QKeySequence(tr("Ctrl+E")), [this]{
        passman::PDPPEntry *named = selectedEntry();
        if (named == nullptr) {
            return;
        }
        EntryEditWidget *di = new EntryEditWidget(named);
        di->setup();

        di->show();

        window->setWindowModified(database->modified);
    });
    toolbar->addAction(editButton);
    toolbar->addSeparator();

    entryMenu = menubar->addMenu(tr("Entry"));

    copyPasswordButton = this->addButton("Copy Password", "edit-copy", "Copy this entry's password. Clipboard will be cleared after a configurable time. (Ctrl+C)", QKeySequence(tr("Ctrl+C")), [this] {
        QClipboard *clip = QApplication::clipboard();

        QTableWidgetItem *item = table->item(table->currentRow(), 0);

        if (item == nullptr) {
            return;
        }

        passman::PDPPEntry *en = database->entryNamed(item->text());
        clip->setText(en->fieldNamed("password")->dataStr());

        QTimer::singleShot(database->clearSecs * 1000, this, [clip] {
            if (qApp->property("verbose").toBool()) {
                qDebug() << "Clearing clipboard";
            }
            clip->setText("");
        });
    });
    entryMenu->addAction(copyPasswordButton);
    toolbar->addAction(copyPasswordButton);

    toolbar->addSeparator();
    toolbar->addAction(configButton);

    toolMenu = menubar->addMenu(tr("Tools"));

    randomButton = passwordGeneratorAction();

    toolMenu->addAction(randomButton);
    toolbar->addAction(randomButton);

    lockButton = addButton("Lock Database", "lock", "Lock the database and require a password to get back in.", QKeySequence(tr("Ctrl+L")), [this] {
        PasswordWidget *passwordWidget = new PasswordWidget(this->database, passman::PasswordOptions::Lock);
        if (!passwordWidget->setup()) {
            return;
        }

        passwordWidget->show();
    });

    toolMenu->addAction(lockButton);
    toolbar->addAction(lockButton);

    aboutMenu = menubar->addMenu(tr("About"));

    aboutButton = this->addButton("About", "help-about", "About passman++", QKeySequence(tr("Ctrl+H")), [] {
        displayErr(tr(("passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help me learn C++.\n\n"
                              "Project repository (Submit bug reports, suggestions, and PRs here, or criticize me for being the master of writing spaghetti code): " + Constants::passmanGithub + " (libpassman GitHub: " + passman::Constants::libpassmanGithub + "\n\n"
                              "FOSS Libraries used: Botan, Qt, libpassman\n\n"
                              "Botan version: " + std::to_string(BOTAN_VERSION_MAJOR) + "." + std::to_string(BOTAN_VERSION_MINOR) + "-" + std::to_string(BOTAN_VERSION_PATCH) + "\n\n"
                              "Current version: " + Constants::passmanVersion + "\n\n"
                              "libpassman version: " + passman::Constants::libpassmanVersion).data()));
    });
    aboutMenu->addAction(aboutButton);

    tipsButton = this->addButton("Tips", "help-hint", "Tips for good password management.", QKeySequence(), [] {
        QDesktopServices::openUrl(QUrl(QString::fromStdString(Constants::passmanGithub) + "blob/main/tips.md"));
    });
    aboutMenu->addAction(tipsButton);
}

bool DatabaseWidget::setup() {
    buttonBox->setStandardButtons(QDialogButtonBox::Ok);
    QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [this] {
        database->save();

        std::exit(0);
    });

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);

    QStringList labels{"Name", "Email", "URL", "Notes"};
    table->setColumnCount(static_cast<int>(labels.size()));
    table->setHorizontalHeaderLabels(labels);

    QObject::connect(table, &QTableWidget::itemDoubleClicked, [this] {
        passman::PDPPEntry *selected = selectedEntry();
        if (selected == nullptr) {
            return;
        }

        EntryEditWidget *di = new EntryEditWidget(selected);
        di->setup();

        di->show();
    });

    fileMenu->addActions(QList<QAction *>{saveButton, saveAsButton});
    fileMenu->addSeparator();

    fileMenu->addAction(configButton);

    entryMenu->addActions(QList<QAction *>{addEButton, delButton, editButton});
    entryMenu->addSeparator();

    entryMenu->addAction(copyPasswordButton);

    aboutMenu->addActions(QList<QAction *>{aboutButton, tipsButton});
    aboutMenu->addAction(tr("About Qt..."), qApp, &QApplication::aboutQt);

    layout->addWidget(table);

    QFont font;
    font.setBold(true);

    preview->addWidget(nameValue, 0, 2);
    preview->addWidget(emailValue, 1, 2);
    preview->addWidget(urlValue, 2, 2);
    preview->addWidget(passValue, 3, 2);

    nameLabel->setFont(font);
    preview->addWidget(nameLabel, 0, 0);

    emailLabel->setFont(font);
    preview->addWidget(emailLabel, 1, 0);

    urlLabel->setFont(font);
    preview->addWidget(urlLabel, 2, 0);

    passLabel->setFont(font);
    preview->addWidget(passLabel, 3, 0);

    preview->setColumnStretch(2, 1);

    layout->addWidget(buttonBox, 2, 0);

    QObject::connect(table, &QTableWidget::itemSelectionChanged, [this] {
        bool anySelected = table->selectedItems().length() > 0;

        prevWidg->setVisible(anySelected);
        if (!anySelected) {
            return layout->removeWidget(prevWidg);
        }

        passman::PDPPEntry *selected = database->entryNamed(table->item(table->currentRow(), 0)->text());
        emailValue->setText(selected->fieldNamed("email")->dataStr());
        urlValue->setText(selected->fieldNamed("url")->dataStr());
        nameValue->setText(selected->name());

        passValue->setText("●●●●●●●●●●●●");

        layout->addWidget(prevWidg, 1, 0);
    });

    passView->setIcon(getIcon("view-visible"));
    passView->setCheckable(true);

    QObject::connect(passView, &QToolButton::clicked, [this](bool checked) {
        passman::PDPPEntry *selected = database->entryNamed(table->item(table->currentRow(), 0)->text());

        if (checked) {
            passValue->setText(selected->fieldNamed("password")->dataStr());
        } else {
            passValue->setText("●●●●●●●●●●●●");
        }
    });

    preview->addWidget(passView, 3, 1);

    this->setLayout(layout);
    return true;
}

void DatabaseWidget::show() {
    redrawTable();

    window->setWidget(this->database->widget, true);
}
