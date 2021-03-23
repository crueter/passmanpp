#ifndef DatabaseWindow_H
#define DatabaseWindow_H
#include <QWidget>
#include <QAction>

#include "mainwindow.hpp"
#include "../util/extra.hpp"
#include "base_widget.hpp"

class QLabel;
class QToolButton;
class QTableWidget;

class Database;
class Entry;

class DatabaseWidget : public BaseWidget
{
public:
    DatabaseWidget(Database *t_database);

    template <typename Func>
    inline QAction *addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func) {
        QAction *action = new QAction(getIcon(tr(icon)), tr(text));

        action->setWhatsThis(tr(whatsThis));
        action->setShortcut(shortcut);
        QObject::connect(action, &QAction::triggered, func);
        return action;
    }

    Entry *selectedEntry();

    void redrawTable();

    bool setup();
    void show();

    QTableWidget *table;

    QAction *saveButton;
    QAction *saveAsButton;
    QMenu *fileMenu;

    QAction *configButton;
    QAction *addEButton;
    QAction *delButton;
    QAction *editButton;

    QMenu *entryMenu;
    QAction *copyPasswordButton;

    QMenu *aboutMenu;
    QAction *aboutButton;
    QAction *tipsButton;

    QWidget *prevWidg;
    QGridLayout *preview;

    QMenu *toolMenu;
    QAction *randomButton;
    QAction *lockButton;

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
