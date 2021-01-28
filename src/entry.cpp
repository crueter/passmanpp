#include <QCheckBox>
#include <QToolButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QTableView>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QSpinBox>

#include "entry.hpp"
#include "gui/random_password_dialog.hpp"
#include "gui/entry_edit_dialog.hpp"

Entry::Entry(QList<Field *> fields, Database *tdb)
    : m_fields(fields)
    , m_database(tdb)
{
    if (fields.empty()) {
        this->setDefaults();
    } else {
        this->m_name = fields[0]->dataStr();
    }
}

void Entry::addField(Field *field) {
    m_fields.push_back(field);
}

bool Entry::removeField(Field *field) {
    return m_fields.removeOne(field);
}

qsizetype Entry::indexOf(Field *field) {
    return m_fields.indexOf(field);
}

Field *Entry::fieldNamed(QString name) {
    for (Field *f : m_fields) {
        if (f->name() == name) {
            return f;
        }
    }
    return nullptr;
}

Field *Entry::fieldAt(const int index) {
    return m_fields[index];
}

const QList<Field *> &Entry::fields() {
    return m_fields;
}

QList<Field *> &Entry::setFields(QList<Field *> &fields) {
    this->m_fields = fields;
    return fields;
}

qsizetype Entry::fieldLength() {
    return m_fields.length();
}

const Database *Entry::database() {
    return m_database;
}

Database *Entry::setDb(Database *database) {
    this->m_database = database;
    return database;
}

const QString &Entry::name() {
    return m_name;
}

QString &Entry::setName(QString &name) {
    this->m_name = name;
    return name;
}

int Entry::edit(QTableWidgetItem *item, QTableWidget *table) {
    EntryEditDialog *di = new EntryEditDialog(this, m_database);
    di->setup();

    return di->show(item, table);
}

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

        return true;
    }
    m_database->redrawTable(item->tableWidget());
    return false;
}

QString Entry::getCreate() {
    QString saveStr = "CREATE TABLE '" + m_fields[0]->dataStr() + "' (";

    const int fieldLen = static_cast<int>(fieldLength());
    auto fieldRange = range(0, fieldLen);

    for (const int i : fieldRange) {
        Field *field = m_fields[i];
        QString fName = field->name();

        const QList<QMetaType::Type> varTypes = {QMetaType::QString, QMetaType::Double, QMetaType::Int, QMetaType::QByteArray};
        const QList<QString> sqlTypes = {"text", "real", "integer", "blob"};

        const qsizetype index = varTypes.indexOf(field->type());
        saveStr += fName.replace('"', '\'') + ' ' + sqlTypes[index];

        if (i != m_fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ")\nINSERT INTO '" + m_fields[0]->dataStr().replace('"', '\'') + "' (";

    for (const int i : fieldRange) {
        Field *field = m_fields[i];
        QString fName = field->name();

        saveStr += fName.replace('"', '\'');

        if (i < m_fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ") VALUES (";

    for (const int i : fieldRange) {
        Field *field = m_fields[i];

        QString quote = field->type() == QMetaType::QString || field->isMultiLine() ? "\"" : "";

        saveStr += quote + field->dataStr().replace('"', '\'').replace('\n', " || char(10) || ") + quote;

        if (i < m_fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ")\n";

    return saveStr;
}

void Entry::setDefaults() {
    for (const QString &s : {"Name", "Email", "URL", "Notes", "Password"}) {
        QMetaType::Type ftype = s == "Notes" ? QMetaType::QByteArray : QMetaType::QString;
        m_fields.push_back(new Field(s, "", ftype));
    }
}
