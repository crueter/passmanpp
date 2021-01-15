#ifndef DatabaseWindow_H
#define DatabaseWindow_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMenuBar>
#include <QLabel>
#include <QToolButton>
#include <QMainWindow>

#include "entry.h"

class DatabaseWindow : public QMainWindow
{
public:
    DatabaseWindow(Database *_database);

    template <typename Func>
    QAction *addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func);

    void setup();
    int exec();
    Entry *getNamed(QTableWidget *table);

    Database *database;

    QDialogButtonBox *ok;
    QGridLayout *layout;
    QTableWidget *table;
    QToolBar *toolbar;

    QAction *saveButton;

    QAction *saveAsButton;

    QMenu *fileMenu;

    QAction *configButton;
    QAction *addEButton;
    QAction *delButton;
    QAction *editButton;

    QAction *randomButton;

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

#endif // DatabaseWindow_H
