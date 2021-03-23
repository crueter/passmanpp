#ifndef EntryEditWidget_H
#define EntryEditWidget_H
#include <QWidget>

#include "mainwindow.hpp"
#include "base_widget.hpp"

class QTableWidget;
class QTableWidgetItem;
class QDoubleSpinBox;
class QTextEdit;
class QLineEdit;
class QDialogButtonBox;
class QCheckBox;

class Database;
class Entry;

class EntryEditWidget : public BaseWidget
{
public:
    EntryEditWidget(Entry *t_entry);

    bool setup();
    void show();

    void addRow(const QString &t_label, QWidget *t_widget, const qsizetype t_index);

    Entry *entry;

    QString origPass;
    QString origName;

    QList<QLineEdit *> lines;
    QList<QCheckBox *> boxes;
    QList<QDoubleSpinBox *> spins;
    QList<QTextEdit *> edits;
};

#endif // EntryEditWidget_H
