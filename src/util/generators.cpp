#include <iterator>
#include <algorithm>
#include <sodium/randombytes.h>
#include <QGridLayout>
#include <QDialogButtonBox>
#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>

#include "generators.h"

QChar randomChar() {
    return (QChar)(0x21U + randombytes_uniform(0x7EU - 0x20U));
}

QString genPass(int length, bool capitals, bool numbers, bool symbols) {
    QString passw;
    QChar csInd;

    for (int i = 0; i < length; ++i) {
        csInd = randomChar();
        while (1) {
            csInd = randomChar();
            if ((i != 0 && csInd == passw[i - 1]) || csInd == '"' || csInd == '\\' || csInd == '\'') {
                continue;
            }
            if (capitals && capital.contains(csInd)) {
                continue;
            }
            if (numbers && number.contains(csInd)) {
                continue;
            }
            if (symbols && symbol.contains(csInd)) {
                continue;
            }
            break;
        }
        passw.append(csInd);
    }
    return passw;
}

void genKey(const QString &path) {
    Botan::AutoSeeded_RNG rng;
    secvec vec = rng.random_vec(128);

    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QDataStream q(&f);

    for (uint8_t v : vec) {
        q << (QChar)v;
    }
    f.close();
}

QString randomPass() {
    QDialog *opt = new QDialog;

    opt->setWindowTitle(tr("Random Password Options"));

    QGridLayout *layout = new QGridLayout;

    QLabel *lengthLabel = new QLabel(tr("Length:"));

    QLineEdit *length = new QLineEdit(opt);
    length->setInputMask("0000");
    length->setCursorPosition(0);

    QCheckBox *capitals = new QCheckBox(tr("Capital Letters"));
    capitals->setCheckState(Qt::CheckState::Checked);

    QCheckBox *numbers = new QCheckBox(tr("Numbers"));
    numbers->setCheckState(Qt::CheckState::Checked);

    QCheckBox *symbols = new QCheckBox(tr("Symbols"));
    symbols->setCheckState(Qt::CheckState::Checked);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(opt);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, opt, &QDialog::accept);
    QObject::connect(buttonBox, &QDialogButtonBox::rejected, opt, &QDialog::reject);

    layout->addWidget(lengthLabel, 0, 0);
    layout->addWidget(length, 0, 1);
    layout->addWidget(capitals, 2, 0, 2, 1);
    layout->addWidget(numbers, 4, 0, 2, 1);
    layout->addWidget(symbols, 6, 0, 2, 1);
    layout->addWidget(buttonBox);

    opt->setLayout(layout);
    int ret = opt->exec();

    if (ret == QDialog::Accepted) {
        return genPass(length->text().toInt(), capitals->checkState() != 2, numbers->checkState() != 2, symbols->checkState() != 2);
    } else {
        return "";
    }
}
