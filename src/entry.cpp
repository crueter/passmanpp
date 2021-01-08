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

void redrawTable(QTableWidget *table, Database *tdb) {
    int j = 0;
    table->setRowCount(tdb->entryLength());
    for (Entry *e : tdb->getEntries()) {
        for (int i = 0; i < e->fieldLength(); ++i) {
            table->setItem(j, i, new QTableWidgetItem(e->fieldAt(i)->dataStr()));
        }
        ++j;
    }
}

Entry::Entry(QList<Field *> fields, Database *tdb) {
    this->fields = fields;
    this->database = tdb;
    if (!fields.empty()) {
        this->name = fields[0]->dataStr();
    }
}

void Entry::addField(Field *field) {
    fields.push_back(field);
}

bool Entry::removeField(Field *field) {
    return fields.removeOne(field);
}

int Entry::indexOf(Field *field) {
    return fields.indexOf(field);
}

Field *Entry::fieldNamed(QString name) {
    for (Field *f : fields) {
        if (f->getName() == name) {
            return f;
        }
    }
    return nullptr;
}

Field *Entry::fieldAt(int index) {
    return fields[index];
}

QList<Field *> &Entry::getFields() {
    return fields;
}

QList<Field *> &Entry::setFields(QList<Field *> &fields) {
    this->fields = fields;
    return fields;
}

int Entry::fieldLength() {
    return fields.length();
}

Database *Entry::getDb() {
    return database;
}

Database *Entry::setDb(Database *database) {
    this->database = database;
    return database;
}

QString &Entry::getName() {
    return name;
}

QString &Entry::setName(QString &name) {
    this->name = name;
    return name;
}

int Entry::edit(QTableWidgetItem *item, QTableWidget *table) {
    EntryEditDialog *di = new EntryEditDialog(this, database);
    di->init();
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
        database->modified = true;

        return true;
    }
    redrawTable(item->tableWidget(), database);
    return false;
}

QString Entry::getCreate() {
    QList<QMetaType::Type> varTypes = {QMetaType::QString, QMetaType::Double, QMetaType::Int, QMetaType::QByteArray};
    QList<QString> sqlTypes = {"text", "real", "integer", "blob"};

    QString saveStr = "CREATE TABLE '" + fields[0]->dataStr() + "' (";

    for (int i = 0; i < fields.size(); ++i) {
        Field *field = fields[i];
        QString fName = field->getName();

        int index = varTypes.indexOf(field->getType());
        saveStr += fName.replace("\"", "'") + " " + sqlTypes[index];

        if (i != fields.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ")\nINSERT INTO '" + fields[0]->dataStr().replace("\"", "'") + "' (";
        }
    }

    for (int i = 0; i < fields.size(); ++i) {
        Field *field = fields[i];
        QString fName = field->getName();

        saveStr += fName.replace("\"", "'");

        if (i < fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ") VALUES (";

    for (int i = 0; i < fields.size(); ++i) {
        Field *field = fields[i];

        QVariant val = field->getData();
        QString quote = "";

        if (field->getType() == QMetaType::QString || field->isMultiLine()) {
            quote = "\"";
        }

        saveStr += quote + val.toString().replace("\"", "'").replace("\n", " || char(10) || ") + quote;

        if (i < fields.size() - 1) {
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
            fields.push_back(new Field(s, "", QMetaType::QByteArray));
        } else {
            fields.push_back(new Field(s, "", QMetaType::QString));
        }
    }
}
