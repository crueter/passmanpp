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
#include "generators.h"

void redrawTable(QTableWidget *table, Database *tdb) {
    int j = 0;
    table->setRowCount(tdb->entries.length());
    for (Entry *e : tdb->entries) {
        for (int i = 0; i < e->fields.size(); ++i) {
            table->setItem(j, i, new QTableWidgetItem(e->fields[i]->data.toString().replace(" || char(10) || ", "\n")));
        }
        ++j;
    }
}

Entry::Entry(QList<Field *> fields, Database *tdb) {
    this->fields = fields;
    this->database = tdb;
    if (!fields.empty()) {
        this->name = fields[0]->name;
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
        if (f->name == name) {
            return f;
        }
    }
    return nullptr;
}

int Entry::edit(QTableWidgetItem *item, QTableWidget *table) {
    bool ok = false;

    QString origPass, origName;
    for (Field *field : fields) {
        if (field->type == QMetaType::QByteArray) {
            QString data = field->data.toString();
            data.replace(" || char(10) || ", "\n");
            field->data = data;
        }
        if (field->name.toLower() == "name") {
            origName = field->data.toString();
        }
        if (field->name.toLower() == "password") {
            origPass = field->data.toString();
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
        switch (field->type) {
            case QMetaType::QString: {
                QLineEdit *edit = new QLineEdit(field->data.toString());
                layout->addRow(field->name + ":", edit);

                if (field->name.toLower() == "name") {
                    edit->setFocus(Qt::FocusReason::MouseFocusReason);
                } else if (field->name.toLower() == "password") {
                    edit->setEchoMode(QLineEdit::Password);

                    QToolButton *random = new QToolButton;
                    random->setIcon(QIcon::fromTheme(tr("roll")));
                    random->setStatusTip(tr("Generate a random password."));

                    connect(random, &QToolButton::clicked, [edit]{
                        QString rand = randomPass();
                        if (rand != "") {
                            edit->setText(rand);
                        }
                    });

                    QToolButton *view = new QToolButton;
                    view->setCheckable(true);
                    view->setIcon(QIcon::fromTheme(tr("view-visible")));
                    random->setStatusTip(tr("Toggle password view."));

                    connect(view, &QToolButton::clicked, [edit](bool checked) {
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
                box->setChecked(field->data.toBool());

                layout->addRow(field->name + ":", box);

                boxes[i] = box;
                break;
            } case QMetaType::Double: {
                QDoubleSpinBox *spin = new QDoubleSpinBox;
                spin->setSingleStep(1.);
                spin->setValue(field->data.toDouble());

                layout->addRow(field->name + ":", spin);

                spins[i] = spin;
                break;
            } case QMetaType::QByteArray: {
                QTextEdit *edit = new QTextEdit(field->data.toString());

                layout->addRow(field->name + ":", edit);

                edits[i] = edit;
                break;
            } default: {
                break;
            }
        }
    }

    QDialogButtonBox *buttonBox = new QDialogButtonBox(opt);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, this, [lines, boxes, spins, edits, origName, origPass, opt, this] {
        for (Field *f : fields) {
            int i = indexOf(f);

            switch(f->type) {
                case QMetaType::QString: {
                    f->data = lines[i]->text();
                    QString txt = lines[i]->text();

                    if (f->name.toLower() == "name") {
                        if (txt == "") {
                            return displayErr("Entry must have a name.");
                        } else if (txt != origName && exists("name", txt)) {
                            return displayErr("An entry named \"" + txt + "\" already exists.");
                        }
                        lines[i]->setFocus(Qt::OtherFocusReason);
                    } else if (f->name.toLower() == "password") {
                        if (txt != origPass && exists("password", txt)) {
                            return displayErr(reuseWarning);
                        } else if (txt.length() < 8) {
                            return displayErr(shortWarning);
                        }
                    }

                    break;
                } case QMetaType::Int: {
                    f->data = boxes[i]->isChecked();
                    break;
                } case QMetaType::Double: {
                    f->data = spins[i]->value();
                    break;
                } case QMetaType::QByteArray: {
                    QString txt = edits[i]->toPlainText();
                    f->data = txt.replace("\n", " || char(10) || ");
                    break;
                } default: {
                    break;
                }
            }
        }
        opt->accept();
    });
    connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, opt, &QDialog::reject);

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

    this->name = fields[0]->data.toString();

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

    QString saveStr = "CREATE TABLE '" + fields[0]->data.toString() + "' (";

    for (int i = 0; i < fields.size(); ++i) {
        Field *field = fields[i];

        int index = varTypes.indexOf(field->type);
        saveStr += field->name.replace("\"", "'") + " " + sqlTypes[index];

        if (i != fields.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ")\nINSERT INTO '" + fields[0]->data.toString().replace("\"", "'") + "' (";
        }
    }

    for (int i = 0; i < fields.size(); ++i) {
        Field *field = fields[i];

        saveStr += field->name.replace("\"", "'");

        if (i < fields.size() - 1) {
            saveStr += ", ";
        }
    }

    saveStr += ") VALUES (";

    for (int i = 0; i < fields.size(); ++i) {
        Field *field = fields[i];

        QVariant val = field->data;
        QString quote = "";

        if (field->type == QMetaType::QString || field->type == QMetaType::QByteArray) {
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
            fields.push_back(new Field(s, QVariant(""), QMetaType::QByteArray));
        } else {
            fields.push_back(new Field(s, QVariant(""), QMetaType::QString));
        }
    }
}
