#include <QLineEdit>
#include <QToolButton>
#include <QAction>

#include "entry_edit_dialog.h"
#include "field.h"
#include "random_password_dialog.h"
#include "../actions/password_generator_action.h"
#include "../actions/password_visible_action.h"

EntryEditDialog::EntryEditDialog(Entry *_entry, Database *_database)
    : entry(_entry)
    , database(_database)
{
    layout = new QFormLayout(this);

    int len = _entry->fieldLength();
    lines.reserve(len);
    boxes.reserve(len);
    spins.reserve(len);
    edits.reserve(len);

    buttonBox = new QDialogButtonBox(this);
}

void EntryEditDialog::setup() {
    for (Field *field : entry->fields()) {
        if (field->type() == QMetaType::QByteArray) {
            QString data = field->dataStr();
            field->setData(data);
        } else if (field->lowerName() == "name") {
            origName = field->dataStr();
        } else if (field->lowerName() == "password") {
            origPass = field->dataStr();
        }
    }

    for (Field *field : entry->fields()) {
        int i = entry->indexOf(field);
        switch (field->type()) {
            case QMetaType::QString: {
                QLineEdit *edit = new QLineEdit(field->dataStr());
                layout->addRow(field->name() + ":", edit);

                if (field->isName()) {
                    edit->setFocus();
                } else if (field->isPass()) {
                    edit->setEchoMode(QLineEdit::Password);

                    passwordVisibleAction(edit, false);

                    passwordGeneratorAction(edit);
                }
                lines[i] = edit;
                break;
            } case QMetaType::Int: {
                QCheckBox *box = new QCheckBox;
                box->setChecked(field->dataStr() != "0");

                layout->addRow(field->name() + ":", box);

                boxes[i] = box;
                break;
            } case QMetaType::Double: {
                QDoubleSpinBox *spin = new QDoubleSpinBox;
                spin->setSingleStep(1.);
                spin->setValue(field->data());

                layout->addRow(field->name() + ":", spin);

                spins[i] = spin;
                break;
            } case QMetaType::QByteArray: {
                QTextEdit *edit = new QTextEdit(field->dataStr());

                layout->addRow(field->name() + ":", edit);

                edits[i] = edit;
                break;
            } default: {
                displayErr(tr("Something has gone horribly wrong. Field type (") + QString((QChar)field->type()) + QString::fromStdString(") is invalid, where valid values are: 2, 6, 10, and 12. Report this issue immediately to" + github));
                break;
            }
        }
    }

    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [this] {
        for (Field *f : entry->fields()) {
            int i = entry->indexOf(f);

            switch(f->type()) {
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

    auto _table = table == nullptr ? item->tableWidget() : table;
    database->redrawTable(_table);

    QString dataS = entry->fieldAt(0)->dataStr();
    entry->setName(dataS);

    return ret;
}
