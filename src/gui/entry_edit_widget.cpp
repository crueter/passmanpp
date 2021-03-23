#include <QDialogButtonBox>
#include <QFormLayout>
#include <QCheckBox>
#include <QDoubleSpinBox>
#include <QTextEdit>
#include <QPushButton>
#include <QLabel>

#include "entry_edit_widget.hpp"
#include "../entry.hpp"
#include "../database.hpp"
#include "../actions/password_generator_action.hpp"
#include "../actions/password_visible_action.hpp"

void EntryEditWidget::addRow(const QString &t_label, QWidget *t_widget, const qsizetype t_index) {
    QLabel *label = new QLabel(t_label);
    layout->addWidget(label, static_cast<int>(t_index), 0);
    layout->addWidget(t_widget, static_cast<int>(t_index), 1);
}

EntryEditWidget::EntryEditWidget(Entry *t_entry)
    : entry(t_entry)
{
    database = entry->database();
    window = database->window;

    buttonBox = new QDialogButtonBox(this);
    layout = new QGridLayout(this);

    title = "Edit Entry " + entry->name();

    qsizetype len = entry->fieldLength();
    lines = QList<QLineEdit *>(len);
    boxes = QList<QCheckBox *>(len);
    spins = QList<QDoubleSpinBox *>(len);
    edits = QList<QTextEdit *>(len);
}

bool EntryEditWidget::setup() {
    for (Field *field : entry->fields()) {
        if (field->type() == QMetaType::QByteArray) {
            field->setData(field->dataStr());
        } else if (field->isName()) {
            origName = field->dataStr();
        } else if (field->isPass()) {
            origPass = field->dataStr();
        }
    }

    for (Field *field : entry->fields()) {
        const qsizetype i = entry->indexOf(field);
        switch (field->type()) {
            case QMetaType::QString: {
                QLineEdit *edit = new QLineEdit(field->dataStr());
                addRow(field->name() + ":", edit, i);

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
                box->setChecked(static_cast<bool>(field->data()));

                addRow(field->name() + ":", box, i);

                boxes[i] = box;
                break;
            } case QMetaType::Double: {
                QDoubleSpinBox *spin = new QDoubleSpinBox;
                spin->setSingleStep(1.);
                spin->setValue(static_cast<double>(field->data()));

                addRow(field->name() + ":", spin, i);

                spins[i] = spin;
                break;
            } case QMetaType::QByteArray: {
                QTextEdit *edit = new QTextEdit(field->dataStr());

                addRow(field->name() + ":", edit, i);

                edits[i] = edit;
                break;
            } default: {
                displayErr(tr("Something has gone horribly wrong. Field type (") + QString(static_cast<QChar>(field->type())) + QString::fromStdString(") is invalid, where valid values are: 2, 6, 10, and 12. Report this issue immediately to" + Constants::github));
                break;
            }
        }
    }

    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    buttonBox->button(QDialogButtonBox::Cancel)->setText(tr("Back"));

    QObject::connect(buttonBox->button(QDialogButtonBox::Ok), &QPushButton::clicked, [this] {
        for (Field *f : entry->fields()) {
            const qsizetype i = entry->indexOf(f);

            switch(f->type()) {
                case QMetaType::QString: {
                    const QString txt = lines[i]->text();

                    if (f->isName()) {
                        if (txt.isEmpty()) {
                            return displayErr("Entry must have a name.");
                        } else if (txt != origName && database->entryNamed(txt) != nullptr) {
                            return displayErr("An entry named \"" + txt + "\" already exists.");
                        }
                        lines[i]->setFocus();
                    } else if (f->isPass()) {
                        if (txt != origPass && database->entryWithPassword(txt) != nullptr) {
                            return displayErr("This password has already been used. DO NOT REUSE PASSWORDS! "
                                              "If somebody gets your password on one account, and you have the same password everywhere, "
                                              "all of your accounts could be compromised and sensitive info could be leaked!");
                        } else if (txt.length() < 8) {
                            return displayErr("Please make your password at least 8 characters. "
                                              "This is the common bare minimum for many websites, "
                                              "and is the shortest password you can have that can't be easily bruteforced.");
                        }
                    }
                    f->setData(txt);

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

        database->modified = true;

        QString dataS = entry->fieldAt(0)->dataStr();
        entry->setName(dataS);

        database->widget->redrawTable();

        window->back();
    });
    QObject::connect(buttonBox->button(QDialogButtonBox::Cancel), &QPushButton::clicked, window, &MainWindow::back);

    layout->addWidget(buttonBox, static_cast<int>(entry->fieldLength()), 1);
    return true;
}

void EntryEditWidget::show() {
    window->setWidget(this);
}
