#ifndef ENTRY_HANDLER_H
#define ENTRY_HANDLER_H
#include <QListWidgetItem>

#include "db.h"

void displayErr(std::string msg);

class EntryHandler : public QWidget {
    Q_OBJECT
public:
    int entryInteract(const char* slot);
    void entryDetails(QString& name, QString& url, QString& email, QString& notes);

    QString randomPass();

    QString addPass();

    int addEntry();

    int displayNames();
    int editEntry();
    int deleteEntry();

    static int _editData(void *list, int count, char **data, char **cols);
public slots:
    static void dispData(QListWidgetItem *item);
    static void editData(QListWidgetItem *item);

    static void delEntry(QListWidgetItem *item);
};

#endif // ENTRY_HANDLER_H
