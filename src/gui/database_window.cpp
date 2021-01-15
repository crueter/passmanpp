#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QDesktopServices>
#include <QPushButton>
#include <QHeaderView>
#include <QToolBar>

#include "database_window.h"
#include "random_password_dialog.h"

template <typename Func>
QAction *DatabaseWindow::addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func) {
    QAction *action = new QAction(QIcon::fromTheme(tr(icon)), tr(text));

    action->setWhatsThis(tr(whatsThis));
    action->setShortcut(shortcut);
    QObject::connect(action, &QAction::triggered, func);
    return action;
}

Entry *DatabaseWindow::getNamed(QTableWidget *table) {
    QTableWidgetItem *item = table->item(table->currentRow(), 0);

    if (item == nullptr) {
        return nullptr;
    }

    QString eName = item->text();

    return database->entryNamed(eName);
}

DatabaseWindow::DatabaseWindow(Database *_database)
    : database(_database)
{
    QWidget *central = new QWidget;
    setCentralWidget(central);

    ok = new QDialogButtonBox(this);
    layout = new QGridLayout(central);
    table = new QTableWidget(this);
    toolbar = new QToolBar;

    saveButton = this->addButton("Save", "document-save", "Save the database as is.", QKeySequence(tr("Ctrl+S")), [this] {
        database->save();
    });
    toolbar->addAction(saveButton);

    saveAsButton = this->addButton("Save as...", "document-save-as", "Save the database to a different location.", QKeySequence(tr("Ctrl+Shift+S")), [this] {
        int br = database->saveAs();
        if (br == 3) {
            displayErr("Invalid save location.");
        } else if (br == 17) {
            displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
        }
    });
    toolbar->addAction(saveAsButton);

    fileMenu = menuBar()->addMenu(tr("File"));

    configButton = this->addButton("Edit Database", "settings-configure", "Edit database options.", QKeySequence(tr("Ctrl+Shift+E")), [this] {
        database->config(false);
    });

    toolbar->addSeparator();

    fileMenu->addAction(configButton);

    randomButton = this->addButton("Password Generator", "roll", "Generate a random password.", QKeySequence(tr("Ctrl+R")), [] {
        RandomPasswordDialog *di = new RandomPasswordDialog;
        di->setup();
        di->show();
    });

    fileMenu->addAction(randomButton);

    addEButton = this->addButton("New", "list-add", "Creates a new entry in the database.", QKeySequence(tr("Ctrl+N")), [this]{
        database->add(table);
    });
    toolbar->addAction(addEButton);

    delButton = this->addButton("Delete", "edit-delete", "Deletes the currently selected entry.", QKeySequence::Delete, [this]{
        Entry *named = getNamed(table);
        if (named == nullptr) {
            return;
        }
        named->del(table->currentItem());
    });
    toolbar->addAction(delButton);

    editButton = this->addButton("Edit", "document-edit", "Edit or view all the information of the current entry.", QKeySequence(tr("Ctrl+E")), [this]{
        Entry *named = getNamed(table);
        if (named == nullptr) {
            return;
        }
        named->edit(table->currentItem());
    });
    toolbar->addAction(editButton);
    toolbar->addSeparator();

    entryMenu = menuBar()->addMenu(tr("Entry"));

    copyPasswordButton = this->addButton("Copy Password", "edit-copy", "Copy this entry's password. Clipboard will be cleared after a configurable time.", QKeySequence(tr("Ctrl+C")), [this] {
        QClipboard *clip = QApplication::clipboard();

        QTableWidgetItem *item = table->item(table->currentRow(), 0);

        if (item == nullptr) {
            return;
        }

        QString eName = item->text();
        Entry *en = database->entryNamed(eName);
        clip->setText(en->fieldNamed("Password")->dataStr());

        QTimer::singleShot(database->clearSecs * 1000, this, [clip] {
            clip->setText("");
        });
    });
    toolbar->addAction(copyPasswordButton);
    toolbar->addSeparator();
    toolbar->addAction(configButton);

    toolbar->addAction(randomButton);

    addToolBar(toolbar);

    entryMenu->addAction(copyPasswordButton);

    aboutButton = this->addButton("About", "help-about", "About passman++", QKeySequence(tr("Ctrl+H")), [] {
        showMessage(info);
    });

    tipsButton = this->addButton("Tips", "help-hint", "Tips for good password management.", QKeySequence(), [] {
        QDesktopServices::openUrl(QUrl(QString::fromStdString(github) + "blob/main/tips.md"));
    });

    aboutMenu = menuBar()->addMenu(tr("About"));
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
    table->setColumnCount(labels.length());
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

        QString eName = table->item(table->currentRow(), 0)->text();

        Entry *selected = database->entryNamed(eName);
        nameValue->setText(selected->name());
        emailValue->setText(selected->fieldNamed("Email")->dataStr());
        urlValue->setText(selected->fieldNamed("URL")->dataStr());

        QString passText;
        for (int i = 0; i < selected->fieldNamed("Password")->dataStr().length(); ++i) {
            passText += "●";
        }
        passValue->setText(passText);

        layout->addWidget(prevWidg, 1, 0);
    });

    passView->setIcon(QIcon::fromTheme("view-visible"));
    passView->setCheckable(true);

    QObject::connect(passView, &QToolButton::clicked, [this](bool checked) {
        QString eName = table->item(table->currentRow(), 0)->text();

        Entry *selected = database->entryNamed(eName);
        QString pass = selected->fieldNamed("Password")->dataStr();

        if (checked) {
            passValue->setText(pass);
        } else {
            QString passText;
            for (int i = 0; i < pass.length(); ++i) {
                passText += "●";
            }
            passValue->setText(passText);
        }
    });

    preview->addWidget(passView, 3, 1);

    this->centralWidget()->setLayout(layout);
    setWindowTitle(tr("Select an entry"));

    resize(800, 450);
}

int DatabaseWindow::exec() {
    database->redrawTable(table);
    show();
    return qApp->exec();
}
