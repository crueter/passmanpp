#ifndef EntryEditWidget_H
#define EntryEditWidget_H
#include <QWidget>

#include <passman/pdpp_entry.hpp>

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

class EntryEditWidget : public BaseWidget
{
public:
    EntryEditWidget(passman::PDPPEntry *t_entry);

    bool setup();
    void show();

    void addRow(const QString &t_label, QWidget *t_widget, const qsizetype t_index);

    passman::PDPPEntry *entry;

    QString origPass;
    QString origName;

    QList<QLineEdit *> lines;
    QList<QCheckBox *> boxes;
    QList<QDoubleSpinBox *> spins;
    QList<QTextEdit *> edits;
};

#endif // EntryEditWidget_H
