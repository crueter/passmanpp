#include <QList>
#include <QMessageBox>
#include <QMetaType>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QString>
#include <QTableWidget>

#include "database.hpp"
#include "entry.hpp"
#include "field.hpp"
#include "util/extra.hpp"

Entry::Entry(QList<Field *> t_fields, Database *t_database)
    : m_fields(t_fields)
    , m_database(t_database)
{
    if (t_fields.empty()) {
        for (const QString &s : {"Name", "Email", "URL", "Notes", "Password"}) {
            QMetaType::Type ftype = s == "Notes" ? QMetaType::QByteArray : QMetaType::QString;
            this->addField(new Field(s, "", ftype));
        }
    } else {
        this->m_name = t_fields[0]->dataStr();
    }
}

// Prompts user for deleting the entry.
bool Entry::del(QTableWidgetItem *item) {
    QTableWidget *table = item->tableWidget();
    if (table == nullptr) {
        return false;
    }

    const QString name = table->item(item->row(), 0)->text();
    QMessageBox delChoice;
    delChoice.setText(tr("Are you sure you want to delete entry \"" + name + "\"? This action is IRREVERSIBLE!"));
    delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delChoice.setDefaultButton(QMessageBox::No);

    if (delChoice.exec() == QMessageBox::Yes) {
        db.exec("DROP TABLE " + name);
        m_database->modified = true;
        m_database->removeEntry(this);
        m_database->redrawTable(item->tableWidget());

        return true;
    }

    m_database->redrawTable(item->tableWidget());
    return false;
}
