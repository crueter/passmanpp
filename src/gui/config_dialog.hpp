#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H
#include <QDialog>
#include <QLineEdit>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
/*
#include <QGridLayout>
#include <QMenuBar>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDialog>
#include <QLabel>
#include <QCheckBox>
*/

class QGridLayout;
class QMenuBar;
class QDialogButtonBox;
class QLabel;
class QCheckBox;

class Database;
class Entry;

class ConfigDialog : public QDialog
{
public:
    ConfigDialog(Database *t_database, const bool t_create);

    inline QLineEdit *lineEdit(const char *text, QString defText, const char *label) {
        QLineEdit *le = new QLineEdit;

        le->setPlaceholderText(tr(text));
        le->setText(defText);

        metaLayout->addRow(tr(label), le);
        return le;
    }

    inline void calcMem() {
        if (hashBox->currentIndex() == 2) {
            const double dispVal = std::round(hashIterBox->value() * 128 * 32768 / 1000000);
            memBox->setValue(static_cast<int>(dispVal));
        }
    }

    QComboBox *comboBox(QList<std::string> vec, const char *label, const int val);
    void updateBoxes(const int index);

    void setup();
    int show();

    Database *database;
    bool create;

    bool paramsChanged;

    QPalette diPalette;
    QColor diC;
    QGridLayout *full;

    QMenuBar *bar;
    QMenu *help;

    QFont bold;
    QFont italic;

    QPalette sectPalette;

    QLabel *metaTitle;
    QFrame *metaWidget;
    QFormLayout *metaLayout;
    QLabel *metaDesc;

    QLineEdit *nameEdit;
    QLineEdit *descEdit;

    QLabel *encTitle;

    QFrame *encWidget;
    QFormLayout *encLayout;
    QLabel *encDesc;

    QComboBox *hmacBox;
    QComboBox *hashBox;
    QComboBox *encryptionBox;

    int iterVal;

    QSpinBox *hashIterBox;
    QSpinBox *memBox;

    QLabel *passTitle;

    QFrame *passWidget;

    QFormLayout *passLayout;

    QLabel *passDesc;

    QLineEdit *pass;

    QLineEdit *keyEdit;

    QPushButton *newKf;

    QPushButton *getKf;

    QDialogButtonBox *keyBox;
    QLabel *keyWarn;

    QLabel *miscTitle;
    QFrame *miscWidget;
    QFormLayout *miscLayout;
    QLabel *miscDesc;

    QCheckBox *compressBox;

    QSpinBox *clearBox;
    QDialogButtonBox *buttonBox;

    QWidget *glw;
    QGridLayout *gl;
};

#endif // CONFIGDIALOG_H
