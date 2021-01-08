#include <QLineEdit>
#include <QToolButton>

#include "entry_edit_dialog.h"
#include "field.h"
#include "random_password_dialog.h"

EntryEditDialog::EntryEditDialog(Entry *entry, Database *database)
{
    this->entry = entry;
    this->database = database;
}

void EntryEditDialog::init() {
    layout = new QFormLayout(this);

    int len = entry->fieldLength();
    lines.reserve(len);
    boxes.reserve(len);
    spins.reserve(len);
    edits.reserve(len);

    buttonBox = new QDialogButtonBox(this);
}

void EntryEditDialog::setup() {
    for (Field *field : entry->getFields()) {
        if (field->getType() == QMetaType::QByteArray) {
            QString data = field->dataStr();
            field->setData(data);
        } else if (field->lowerName() == "name") {
            origName = field->dataStr();
        } else if (field->lowerName() == "password") {
            origPass = field->dataStr();
        }
    }

    for (Field *field : entry->getFields()) {
        int i = entry->indexOf(field);
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
                        RandomPasswordDialog *di = new RandomPasswordDialog;
                        di->init();
                        di->setup();
                        QString rand = di->show();
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

                    QDialogButtonBox *passButtons = new QDialogButtonBox(this);
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
                displayErr(tr("Something has gone horribly wrong. Field type (") + QString((QChar)field->getType()) + QString::fromStdString(") is invalid, where valid values are: 2, 6, 10, and 12. Report this issue immediately to" + github));
                break;
            }
        }
    }

    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [this] {
        for (Field *f : entry->getFields()) {
            int i = entry->indexOf(f);

            switch(f->getType()) {
                case QMetaType::QString: {
                    f->setData(lines[i]->text());
                    QString txt = lines[i]->text();

                    if (f->isName()) {
                        if (txt.isEmpty()) {
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
                    f->setData(edits[i]->toPlainText());
                    break;
                } default: {
                    break;
                }
            }
        }
        accept();
    });
    QObject::connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, this, &QDialog::reject);

    layout->addWidget(buttonBox);
}

int EntryEditDialog::show(QTableWidgetItem *item, QTableWidget *table) {
    int ret = exec();

    if (ret == QDialog::Rejected) {
        return false;
    }

    database->modified = true;

    if (table == nullptr) {
        redrawTable(item->tableWidget(), database);
    } else {
        redrawTable(table, database);
    }

    QString dataS = entry->fieldAt(0)->dataStr();
    entry->setName(dataS);

    return ret;
}
