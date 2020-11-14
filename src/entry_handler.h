#ifndef ENTRY_HANDLER_H
#define ENTRY_HANDLER_H
#include <QListWidgetItem>

#include "db.h"

void displayErr(std::string msg);

class EntryHandler : public QWidget {
    Q_OBJECT
public:
    int entryInteract();
    void entryDetails(QString& name, QString& url, QString& email, QString& password, QString& notes);

    QString randomPass();

    int addEntry();

    int displayData(QListWidgetItem *item);
    int editEntry(QListWidgetItem *item);
    bool deleteEntry(QListWidgetItem *item);

    static int _editData(void *list, int count, char **data, char **cols);
public slots:
    static void dispData(QListWidgetItem *item);
};

#endif // ENTRY_HANDLER_H
