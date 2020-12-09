#ifndef ENTRY_HANDLER_H
#define ENTRY_HANDLER_H
#include <QListWidgetItem>
#include <QPushButton>
#include <QLabel>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QMenuBar>

#include "../database.h"

void displayErr(std::string msg);

class EntryHandler : public QWidget {
public:
    int entryInteract(Database db);
    bool entryDetails(QString& name, QString& url, QString& email, QString& password, QString& notes);

    QString randomPass();

    int addEntry(QListWidget *item, Database db);
    template <typename Func>
    QAction *addButton(QIcon icon, const char *whatsThis, QKeySequence shortcut, Func func);

    int editEntry(QListWidgetItem *item, Database db);
    bool deleteEntry(QListWidgetItem *item, Database db);

    static int _editData(void *, int, char **data, char **);
};

#endif // ENTRY_HANDLER_H
