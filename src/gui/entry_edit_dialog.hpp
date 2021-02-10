#ifndef ENTRYEDITDIALOG_H
#define ENTRYEDITDIALOG_H
#include <QDialog>

class QTableWidget;
class QTableWidgetItem;
class QFormLayout;
class QDoubleSpinBox;
class QTextEdit;
class QLineEdit;
class QDialogButtonBox;
class QCheckBox;

class Database;
class Entry;

class EntryEditDialog : public QDialog
{
public:
    EntryEditDialog(Entry *t_entry, Database *m_database);

    void setup();
    int show(QTableWidgetItem *item, QTableWidget *table);

    Entry *entry;
    Database *database;

    QString origPass;
    QString origName;

    QFormLayout *layout;
    QList<QLineEdit *> lines;
    QList<QCheckBox *> boxes;
    QList<QDoubleSpinBox *> spins;
    QList<QTextEdit *> edits;

    QDialogButtonBox *buttonBox;

};

#endif // ENTRYEDITDIALOG_H
