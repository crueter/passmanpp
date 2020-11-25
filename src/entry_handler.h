#ifndef ENTRY_HANDLER_H
#define ENTRY_HANDLER_H
#include <QInputDialog>

#include "db.h"

void displayErr(std::string msg);

class EntryHandler : public QWidget {
    Q_OBJECT
public:
    int entryInteract();
    bool entryDetails(QString& name, QString& url, QString& email, QString& password, QString& notes);

    bool create(std::string path);
    QString randomPass();

    int addEntry(QListWidget *list);
    template <typename Func>
    QAction *addButton(QIcon icon, QString statusTip, QKeySequence shortcut, Func func);

    int editEntry(QListWidgetItem *item);
    bool deleteEntry(QListWidgetItem *item);

    static int _editData(void *, int, char **data, char **);
};

#endif // ENTRY_HANDLER_H
