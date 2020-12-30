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
#include "util/generators.h"

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
    bool ok = false;

    QString origPass, origName;
    for (Field *field : fields) {
        if (field->getType() == QMetaType::QByteArray) {
            QString data = field->dataStr();
            data.replace(" || char(10) || ", "\n");
            field->setData(data);
        } else if (field->lowerName() == "name") {
            origName = field->dataStr();
        } else if (field->lowerName() == "password") {
            origPass = field->dataStr();
        }
    }

    QDialog *opt = new QDialog;
    QFormLayout *layout = new QFormLayout;
    int len = fields.length();
    QList<QLineEdit *> lines(len);
    QList<QCheckBox *> boxes(len);
    QList<QDoubleSpinBox *> spins(len);
    QList<QTextEdit *> edits(len);

    for (Field *field : fields) {
        int i = indexOf(field);
        switch (field->getType()) {
            case QMetaType::QString: {
                QLineEdit *edit = new QLineEdit(field->dataStr());
                layout->addRow(field->getName() + ":", edit);

                if (field->isName()) {
                    edit->setFocus();
                } else if (field->isPass()) {
                    edit->setEchoMode(QLineEdit::Password);

                    QToolButton *random = new QToolButton;
                    random->setIcon(QIcon::fromTheme(tr("roll")));
                    random->setStatusTip(tr("Generate a random password."));

                    QObject::connect(random, &QToolButton::clicked, [edit]{
                        QString rand = randomPass();
                        if (rand != "") {
                            edit->setText(rand);
                        }
                    });

                    QToolButton *view = new QToolButton;
                    view->setCheckable(true);
                    view->setIcon(QIcon::fromTheme(tr("view-visible")));
                    random->setStatusTip(tr("Toggle password view."));

                    QObject::connect(view, &QToolButton::clicked, [edit](bool checked) {
                        QLineEdit::EchoMode echo;
                        if (checked) {
                            echo = QLineEdit::Normal;
                        } else {
                            echo = QLineEdit::Password;
                        }
                        edit->setEchoMode(echo);
                    });

                    QDialogButtonBox *passButtons = new QDialogButtonBox(opt);
                    passButtons->addButton(random, QDialogButtonBox::ActionRole);
                    passButtons->addButton(view, QDialogButtonBox::ActionRole);

                    layout->addWidget(passButtons);
                }
                lines[i] = edit;
                break;
            } case QMetaType::Int: {
                QCheckBox *box = new QCheckBox;
                box->setChecked(field->getData().toBool());

                layout->addRow(field->getName() + ":", box);

                boxes[i] = box;
                break;
            } case QMetaType::Double: {
                QDoubleSpinBox *spin = new QDoubleSpinBox;
                spin->setSingleStep(1.);
                spin->setValue(field->getData().toDouble());

                layout->addRow(field->getName() + ":", spin);

                spins[i] = spin;
                break;
            } case QMetaType::QByteArray: {
                QTextEdit *edit = new QTextEdit(field->dataStr());

                layout->addRow(field->getName() + ":", edit);

                edits[i] = edit;
                break;
            } default: {
                qDebug() << "Something has gone horribly wrong. Field type is invalid:" << field->getType() << " where valid values are: 2, 6, 10, and 12. Report this issue immediately to" << QString::fromStdString(github);
                break;
            }
        }
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(opt);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [lines, boxes, spins, edits, origName, origPass, opt, this] {
        for (Field *f : fields) {
            int i = indexOf(f);

            switch(f->getType()) {
                case QMetaType::QString: {
                    f->setData(lines[i]->text());
                    QString txt = lines[i]->text();

                    if (f->isName()) {
                        if (txt == "") {
                            return displayErr("Entry must have a name.");
                        } else if (txt != origName && exists("name", txt)) {
                            return displayErr("An entry named \"" + txt + "\" already exists.");
                        }
                        lines[i]->setFocus();
                    } else if (f->isPass()) {
                        if (txt != origPass && exists("password", txt)) {
                            return displayErr(reuseWarning);
                        } else if (txt.length() < 8) {
                            return displayErr(shortWarning);
                        }
                    }

                    break;
                } case QMetaType::Int: {
                    f->setData(boxes[i]->isChecked());
                    break;
                } case QMetaType::Double: {
                    f->setData(spins[i]->value());
                    break;
                } case QMetaType::QByteArray: {
                    QString txt = edits[i]->toPlainText();
                    f->setData(txt.replace("\n", " || char(10) || "));
                    break;
                } default: {
                    break;
                }
            }
        }
        opt->accept();
    });
    QObject::connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, opt, &QDialog::reject);

    layout->addWidget(buttonBox);

    opt->setLayout(layout);

    int ret = opt->exec();

    if (ret == QDialog::Rejected) {
        return false;
    }

    database->modified = true;

    if (table == nullptr) {
        redrawTable(item->tableWidget(), database);
    } else {
        redrawTable(table, database);
    }

    this->name = fields[0]->dataStr();

    return ok;
}

int Entry::del(QTableWidgetItem *item) {
    QString name = item->tableWidget()->item(item->row(), 0)->text();
    QMessageBox delChoice;
    delChoice.setText(tr(std::string("Are you sure you want to delete entry \"" + name.toStdString() + "\"? This action is IRREVERSIBLE!").data()));
    delChoice.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    delChoice.setDefaultButton(QMessageBox::No);
    int ret = delChoice.exec();

    if (ret == QMessageBox::Yes) {
        db.exec("DROP TABLE " + name);
        database->modified = true;

        std::cout << "Entry \"" << name.toStdString() << "\" successfully deleted." << std::endl;
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

        saveStr += quote + val.toString().replace("\"", "'") + quote;

        if (i < fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ")\n";

    return saveStr;
}

void Entry::setDefaults() {
    QStringList names = {"Name", "Email", "URL", "Password", "Notes"};
    for (const QString &s : names) {
        if (s == "Notes") {
            fields.push_back(new Field(s, "", QMetaType::QByteArray));
        } else {
            fields.push_back(new Field(s, "", QMetaType::QString));
        }
    }
}
