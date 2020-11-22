#include <QListWidgetItem>
#include <QFormLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QStringList>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QInputDialog>
#include <sodium.h>

#include "manager.h"
#include "pdpp_handler.h"
#include "file_handler.h"

bool open(std::string path) {
    if (std::experimental::filesystem::exists(path)) {
        pdb.open(path, std::ios_base::binary | std::ios_base::out | std::ios_base::in | std::ios_base::ate);
        std::vector<std::string> r;
        try {
            r = decryptData(path, " to login");
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        std::string line;
        std::istringstream iss(std::any_cast<std::string>(r[0]));
        while (std::getline(iss, line))
            exec(line);
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

bool create(std::string path) {
    pdb.open(path);

    std::string pw = QInputDialog::getText(nullptr, QWidget::tr("Create Database"), QWidget::tr("Welcome! To start, please set a master password: "), QLineEdit::Password).toStdString();
    QDialog *di = new QDialog;
    di->setWindowTitle("Database Options");

    QFormLayout *layout = new QFormLayout;

    auto tr = [=](const char* str) {
        return QWidget::tr(str);
    };

    auto comboBox = [layout, tr](std::vector<const char *> vec, const char * label) -> QComboBox* {
        QComboBox *box = new QComboBox;
        QStringList list = QStringList(vec.begin(), vec.end());
        box->addItems(list);
        box->setCurrentIndex(0);
        layout->addRow(tr(label), box);
        return box;
    };

    QLineEdit *name = new QLineEdit;
    name->setPlaceholderText(tr("Name"));
    name->setMaxLength(255);

    QTextEdit *desc = new QTextEdit;
    desc->setPlaceholderText(tr("Description"));

    layout->addRow(tr("Database Name:"), name);
    layout->addRow(tr("Database Description"), desc);

    QComboBox *checksumBox = comboBox(checksumMatch, "Checksum Function:");
    QComboBox *derivBox = comboBox(derivMatch, "Key Derivation Function:");
    QComboBox *hashBox = comboBox(hashMatch, "Password Hashing Function:");
    QComboBox *encryptionBox = comboBox(encryptionMatch, "Data Encryption Function:");

    QSlider *hashIterSlider = new QSlider(Qt::Horizontal);
    hashIterSlider->setRange(1, 255);
    hashIterSlider->setValue(8);

    QLabel *hashIterLabel = new QLabel(tr("Password Hashing Iterations:"));
    QWidget::connect(hashIterSlider, &QSlider::valueChanged, [hashIterLabel](int value) {
        hashIterLabel->setText(QString::fromStdString(split(hashIterLabel->text().toStdString(), ':')[0] + ": " + std::to_string(value)));
    });

    layout->addRow(hashIterLabel, hashIterSlider);

    std::string keyFileName;
    bool useKeyFile = false;

    QPushButton *keyFile = new QPushButton(tr("Browse..."));
    QWidget::connect(keyFile, &QPushButton::clicked, [keyFileName, useKeyFile, keyFile]() mutable {
        FileHandler *fh = new FileHandler;
        keyFileName = fh->newKeyFile();
        if (keyFileName != "")
            useKeyFile = true;
        keyFile->setText(QString::fromStdString(keyFileName));
    });

    layout->addRow(tr("Key File:"), keyFile);

    di->setLayout(layout);
    di->exec();

    int uuidLen = randombytes_uniform(80);
    Botan::AutoSeeded_RNG rng;
    Botan::secure_vector<uint8_t> uuid = rng.random_vec(uuidLen);

    int arc = exec("CREATE TABLE data (name text, email text, url text, notes text, password text)");
    saveSt();

    encryptData(path, checksumBox->currentIndex() + 1, derivBox->currentIndex() + 1, hashBox->currentIndex() + 1, hashIterSlider->value(), useKeyFile, encryptionBox->currentIndex() + 1, uuid, name->text().toStdString(), desc->toPlainText().toStdString(), stList, pw);

    return arc;
}

/*bool save(std::string path) {
    if (!modified) std::cout << "The database is already up to date." << std::endl;
    else {
        std::vector<std::string> mp;
        try {
            mp = getmpass(" to save", path);
        } catch (std::exception& e) {
            displayErr(e.what());
            return false;
        }
        encrypt(mp[1], path);
        modified = false;
    }
    return true;
}*/
bool save(std::string path) {
    std::cout << "soon" << std::endl;
}
