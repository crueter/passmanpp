#ifndef DatabaseWindow_H
#define DatabaseWindow_H

#include <QDialog>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QMenuBar>
#include <QLabel>
#include <QToolButton>
#include <QMainWindow>

#include "entry.hpp"

class DatabaseWindow : public QMainWindow
{
public:
    DatabaseWindow(Database *t_database);

    template <typename Func>
    QAction *addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func);

    void setup();
    int exec();
    Entry *getNamed(QTableWidget *t_table);

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

    QMenu *entryMenu;

    QAction *copyPasswordButton;
    QAction *aboutButton;

    QAction *tipsButton;

    QMenu *aboutMenu;
    QWidget *prevWidg;
    QGridLayout *preview;

    QMenu *toolMenu;
    QAction *randomButton;

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
