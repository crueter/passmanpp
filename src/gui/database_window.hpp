#ifndef DatabaseWindow_H
#define DatabaseWindow_H
#include <QDialog>
#include <QMainWindow>
#include <QAction>

class QDialogButtonBox;
class QGridLayout;
class QMenuBar;
class QLabel;
class QToolButton;
class QTableWidget;

class Database;
class Entry;

class DatabaseWindow : public QMainWindow
{
public:
    DatabaseWindow(Database *t_database);

    template <typename Func>
    inline QAction *addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func) {
        QAction *action = new QAction(QIcon::fromTheme(tr(icon)), tr(text));

        action->setWhatsThis(tr(whatsThis));
        action->setShortcut(shortcut);
        QObject::connect(action, &QAction::triggered, func);
        return action;
    }

    Entry *getNamed(QTableWidget *t_table);

    void setup();
    int exec();

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

    QMenu *aboutMenu;

    QAction *aboutButton;
    QAction *tipsButton;

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
