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

#include "entry.h"
#include "gui/random_password_dialog.h"
#include "gui/entry_edit_dialog.h"

Entry::Entry(QList<Field *> fields, Database *tdb)
    : _fields(fields)
    , _database(tdb)
{
    if (!fields.empty()) {
        this->_name = fields[0]->dataStr();
    }
}

void Entry::addField(Field *field) {
    _fields.push_back(field);
}

bool Entry::removeField(Field *field) {
    return _fields.removeOne(field);
}

int Entry::indexOf(Field *field) {
    return _fields.indexOf(field);
}

Field *Entry::fieldNamed(QString name) {
    for (Field *f : _fields) {
        if (f->name() == name) {
            return f;
        }
    }
    return nullptr;
}

Field *Entry::fieldAt(int index) {
    return _fields[index];
}

const QList<Field *> &Entry::fields() {
    return _fields;
}

QList<Field *> &Entry::setFields(QList<Field *> &fields) {
    this->_fields = fields;
    return fields;
}

int Entry::fieldLength() {
    return _fields.length();
}

const Database *Entry::database() {
    return _database;
}

Database *Entry::setDb(Database *database) {
    this->_database = database;
    return database;
}

const QString &Entry::name() {
    return _name;
}

QString &Entry::setName(QString &name) {
    this->_name = name;
    return name;
}

int Entry::edit(QTableWidgetItem *item, QTableWidget *table) {
    EntryEditDialog *di = new EntryEditDialog(this, _database);
    di->setup();
    int ok = di->show(item, table);

    return ok;
}

bool Entry::del(QTableWidgetItem *item) {
    QTableWidget *table = item->tableWidget();
    if (table == nullptr) {
        return false;
    }

    QString name = table->item(item->row(), 0)->text();
    QMessageBox delChoice;
    delChoice.setText(tr("Are you sure you want to delete entry \"" + name + "\"? This action is IRREVERSIBLE!"));
    delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delChoice.setDefaultButton(QMessageBox::No);
    int ret = delChoice.exec();

    if (ret == QMessageBox::Yes) {
        db.exec("DROP TABLE " + name);
        _database->modified = true;

        return true;
    }
    _database->redrawTable(item->tableWidget());
    return false;
}

QString Entry::getCreate() {
    QList<QMetaType::Type> varTypes = {QMetaType::QString, QMetaType::Double, QMetaType::Int, QMetaType::QByteArray};
    QList<QString> sqlTypes = {"text", "real", "integer", "blob"};

    QString saveStr = "CREATE TABLE '" + _fields[0]->dataStr() + "' (";

    for (int i = 0; i < _fields.size(); ++i) {
        Field *field = _fields[i];
        QString fName = field->name();

        int index = varTypes.indexOf(field->type());
        saveStr += fName.replace("\"", "'") + " " + sqlTypes[index];

        if (i != _fields.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ")\nINSERT INTO '" + _fields[0]->dataStr().replace("\"", "'") + "' (";
        }
    }

    for (int i = 0; i < _fields.size(); ++i) {
        Field *field = _fields[i];
        QString fName = field->name();

        saveStr += fName.replace("\"", "'");

        if (i < _fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ") VALUES (";

    for (int i = 0; i < _fields.size(); ++i) {
        Field *field = _fields[i];

        QString val = field->dataStr();
        QString quote = "";

        if (field->type() == QMetaType::QString || field->isMultiLine()) {
            quote = "\"";
        }

        saveStr += quote + val.replace("\"", "'").replace("\n", " || char(10) || ") + quote;

        if (i < _fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ")\n";

    return saveStr;
}

void Entry::setDefaults() {
    QStringList names = {"Name", "Email", "URL", "Notes", "Password"};
    for (const QString &s : names) {
        if (s == "Notes") {
            _fields.push_back(new Field(s, "", QMetaType::QByteArray));
        } else {
            _fields.push_back(new Field(s, "", QMetaType::QString));
        }
    }
}
