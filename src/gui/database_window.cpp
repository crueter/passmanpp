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

#include "database_window.hpp"
#include "random_password_dialog.hpp"
#include "../entry.hpp"
#include "../database.hpp"

Entry *DatabaseWindow::getNamed(QTableWidget *t_table) {
    QTableWidgetItem *item = t_table->item(t_table->currentRow(), 0);

    if (item == nullptr) {
        return nullptr;
    }

    return database->entryNamed(item->text());
}

DatabaseWindow::DatabaseWindow(Database *t_database)
    : database(t_database)
{
    QWidget *central = new QWidget;
    setCentralWidget(central);

    ok = new QDialogButtonBox(this);
    layout = new QGridLayout(central);
    table = new QTableWidget(this);
    toolbar = new QToolBar;

    saveButton = this->addButton("Save", "document-save", "Save the database as is. (Ctrl+S)", QKeySequence(tr("Ctrl+S")), [this] {
        database->save();
        this->setWindowModified(false);
    });
    toolbar->addAction(saveButton);

    saveAsButton = this->addButton("Save as...", "document-save-as", "Save the database to a different location. (Ctrl+Shift+S)", QKeySequence(tr("Ctrl+Shift+S")), [this] {
        int br = database->saveAs();
        if (br == 3) {
            displayErr("Invalid save location.");
        } else if (br == 17) {
            displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
        } else {
            this->setWindowModified(false);
        }
    });
    toolbar->addAction(saveAsButton);

    fileMenu = menuBar()->addMenu(tr("File"));

    configButton = this->addButton("Edit Database", "settings-configure", "Edit database options. (Ctrl+Shift+E)", QKeySequence(tr("Ctrl+Shift+E")), [this] {
        database->config(false);
        this->setWindowModified(false);
    });

    toolbar->addSeparator();

    fileMenu->addAction(configButton);

    addEButton = this->addButton("New", "list-add", "Creates a new entry in the database. (Ctrl+N)", QKeySequence(tr("Ctrl+N")), [this]{
        database->add(table);
        this->setWindowModified(database->modified);
    });
    toolbar->addAction(addEButton);

    delButton = this->addButton("Delete", "edit-delete", "Deletes the currently selected entry. (Delete)", QKeySequence::Delete, [this]{
        Entry *named = getNamed(table);
        if (named == nullptr) {
            return;
        }
        named->del(table->currentItem());

        this->setWindowModified(database->modified);
    });
    toolbar->addAction(delButton);

    editButton = this->addButton("Edit", "document-edit", "Edit or view all the information of the current entry. (Ctrl+E)", QKeySequence(tr("Ctrl+E")), [this]{
        Entry *named = getNamed(table);
        if (named == nullptr) {
            return;
        }
        named->edit(table->currentItem());

        this->setWindowModified(database->modified);
    });
    toolbar->addAction(editButton);
    toolbar->addSeparator();

    entryMenu = menuBar()->addMenu(tr("Entry"));

    copyPasswordButton = this->addButton("Copy Password", "edit-copy", "Copy this entry's password. Clipboard will be cleared after a configurable time. (Ctrl+C)", QKeySequence(tr("Ctrl+C")), [this] {
        QClipboard *clip = QApplication::clipboard();

        QTableWidgetItem *item = table->item(table->currentRow(), 0);

        if (item == nullptr) {
            return;
        }

        Entry *en = database->entryNamed(item->text());
        clip->setText(en->fieldNamed("password")->dataStr());

        QTimer::singleShot(database->clearSecs * 1000, this, [clip] {
            clip->setText("");
        });
    });
    toolbar->addAction(copyPasswordButton);
    toolbar->addSeparator();
    toolbar->addAction(configButton);

    toolMenu = menuBar()->addMenu(tr("Tools"));

    randomButton = this->addButton("Password Generator", "roll", "Generate a random password. (Ctrl+R)", QKeySequence(tr("Ctrl+R")), [] {
        RandomPasswordDialog *di = new RandomPasswordDialog;
        di->setup();
        di->show();
    });

    toolMenu->addAction(randomButton);

    toolbar->addAction(randomButton);

    addToolBar(toolbar);

    entryMenu->addAction(copyPasswordButton);

    aboutMenu = menuBar()->addMenu(tr("About"));

    aboutButton = this->addButton("About", "help-about", "About passman++", QKeySequence(tr("Ctrl+H")), [] {
        showMessage(tr(("passman++ is a simple, minimal, yet powerful, secure command-line password manager. Written in C++, passman++ is a complete rewrite of my original passman project (https://github.com/binex-dsk/PyPassMan) intended to be far more secure, feature-rich, fast, and most of all, to help me learn C++.\n"
                              "Project repository (Submit bug reports, suggestions, and PRs here, or criticize me for being the master of writing spaghetti code): " + Constants::github + "\n"
                              "FOSS Libraries used: Botan, Qt, libsodium\n"
                              "Botan version: " + std::to_string(BOTAN_VERSION_MAJOR) + "." + std::to_string(BOTAN_VERSION_MINOR) + "-" + std::to_string(BOTAN_VERSION_PATCH) + "\n"
                              "Current version: " + Constants::passmanVersion).data()));
    });
    aboutMenu->addAction(aboutButton);

    tipsButton = this->addButton("Tips", "help-hint", "Tips for good password management.", QKeySequence(), [] {
        QDesktopServices::openUrl(QUrl(QString::fromStdString(Constants::github) + "blob/main/tips.md"));
    });
    aboutMenu->addAction(tipsButton);

    prevWidg = new QWidget;
    preview = new QGridLayout(prevWidg);

    nameValue = new QLabel(prevWidg);
    emailValue = new QLabel(prevWidg);
    urlValue = new QLabel(prevWidg);
    passValue = new QLabel(prevWidg);
    nameLabel = new QLabel(tr("Name"), prevWidg);
    emailLabel = new QLabel(tr("Email"), prevWidg);
    urlLabel = new QLabel(tr("URL"), prevWidg);
    passLabel = new QLabel(tr("Password"), prevWidg);
    passView = new QToolButton(prevWidg);
}

void DatabaseWindow::setup() {
    ok->setStandardButtons(QDialogButtonBox::Ok);
    QObject::connect(ok->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, &QMainWindow::close);

    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);

    QStringList labels{"Name", "Email", "URL", "Notes"};
    table->setColumnCount(static_cast<int>(labels.size()));
    table->setHorizontalHeaderLabels(labels);

    QObject::connect(table, &QTableWidget::itemDoubleClicked, [this](QTableWidgetItem *item) {
        getNamed(item->tableWidget())->edit(item);
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

    layout->addWidget(ok, 2, 0);

    QObject::connect(table, &QTableWidget::itemSelectionChanged, [this] {
        bool anySelected = table->selectedItems().length() > 0;

        prevWidg->setVisible(anySelected);
        if (!anySelected) {
            return layout->removeWidget(prevWidg);
        }

        Entry *selected = database->entryNamed(table->item(table->currentRow(), 0)->text());
        nameValue->setText(selected->name());
        emailValue->setText(selected->fieldNamed("email")->dataStr());
        urlValue->setText(selected->fieldNamed("url")->dataStr());

        passValue->setText("●●●●●●●●●●●●");

        layout->addWidget(prevWidg, 1, 0);
    });

    passView->setIcon(QIcon::fromTheme("view-visible"));
    passView->setCheckable(true);

    QObject::connect(passView, &QToolButton::clicked, [this](bool checked) {
        Entry *selected = database->entryNamed(table->item(table->currentRow(), 0)->text());

        if (checked) {
            passValue->setText(selected->fieldNamed("password")->dataStr());
        } else {
            passValue->setText("●●●●●●●●●●●●");
        }
    });

    preview->addWidget(passView, 3, 1);

    this->centralWidget()->setLayout(layout);
    setWindowTitle(tr("Select an entry [*]"));

    resize(800, 450);
}

int DatabaseWindow::exec() {
    database->redrawTable(table);
    show();
    return qApp->exec();
}
