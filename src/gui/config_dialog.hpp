#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H
#include <QGridLayout>
#include <QMenuBar>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDialogButtonBox>

#include "entry.hpp"

class ConfigDialog : QDialog
{
public:
    ConfigDialog(Database *t_database, const bool t_create);
    QLineEdit *lineEdit(const char *text, QString defText, const char *label);
    QComboBox *comboBox(QList<std::string> vec, const char *label, const int val);
    void calcMem();
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
