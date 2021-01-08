#ifndef ENTRYEDITDIALOG_H
#define ENTRYEDITDIALOG_H

#include <QDialog>
#include <QTableWidgetItem>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QDialogButtonBox>

#include "entry.h"

class EntryEditDialog : public QDialog
{
public:
    EntryEditDialog(Entry *entry, Database *database);

    void init();
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
