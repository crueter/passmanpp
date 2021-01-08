#ifndef DATABASEEDITDIALOG_H
#define DATABASEEDITDIALOG_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMenuBar>
#include <QLabel>
#include <QToolButton>

#include "entry.h"

class DatabaseEditDialog : public QDialog
{
public:
    DatabaseEditDialog(Database *database);

    template <typename Func>
    QAction *addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func);

    void init();
    void setup();
    int show();
    Entry *getNamed(QTableWidget *table);

    Database *database;

    QDialogButtonBox *ok;
    QGridLayout *layout;
    QTableWidget *table;
    QMenuBar *bar;

    QAction *saveButton;

    QAction *saveAsButton;

    QMenu *fileMenu;

    QAction *configButton;
    QAction *addEButton;
    QAction *delButton;
    QAction *editButton;

    QMenu *entryMenu;

    QAction *copyPasswordButton;
    QAction *aboutButton;

    QAction *tipsButton;

    QMenu *aboutMenu;
    QWidget *prevWidg;
    QGridLayout *preview;

    QLabel *nameValue;
    QLabel *emailValue;
    QLabel *urlValue;
    QLabel *passValue;
    QLabel *nameLabel;
    QLabel *emailLabel;
    QLabel *urlLabel;
    QLabel *passLabel;
    QToolButton *passView;
};

#endif // DATABASEEDITDIALOG_H
