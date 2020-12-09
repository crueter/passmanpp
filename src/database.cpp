#include <QComboBox>
#include <QSpinBox>
#include <QUrl>
#include <QDesktopServices>
#include <QInputDialog>

#include "handlers/entry_handler.h"
#include "util/generators.h"

Database::Database() {}
std::string kfp = "";

void showMessage(std::string msg) {
    QMessageBox box;
    box.setText(QString::fromStdString(msg));
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

std::string getCS(uint8_t cs, uint8_t encr) {
    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encr), Botan::ENCRYPTION);
    std::string checksumChoice = std::string(checksumMatch[cs]);

    if (checksumChoice != "SHA-512") {
        checksumChoice += "(" + std::to_string(enc->maximum_keylength() * 8) + ")";
    }
    return checksumChoice;
}

std::string Database::getPw(std::string password) {
    std::string checksumChoice = getCS(checksum, encryption);
    std::string hashChoice = hashMatch[hash];

    if (hashChoice != "No hashing, only derivation") {
        std::unique_ptr<Botan::PasswordHashFamily> pfHash = Botan::PasswordHashFamily::create(hashChoice);
        std::unique_ptr<Botan::PasswordHash> pHash;
        if (hashChoice == "Argon2id") {
            pHash = pfHash->from_params(80000, hashIters, 1);
        } else {
            pHash = pfHash->from_params(hashIters);
        }

        Botan::secure_vector<uint8_t> ptr(1024);
        pHash->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), iv.data(), iv.size());
        password = toStr(ptr);
    }

    std::string derivChoice = derivMatch[deriv];

    Botan::secure_vector<uint8_t> ptr(32);
    std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create(derivChoice + "(" + checksumChoice + ")")->default_params();

    ph->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), iv.data(), iv.size());

    return toStr(ptr);
}

void Database::encrypt(std::string password) {
    std::ofstream pd(path, std::ios_base::binary | std::ios_base::trunc);
    pd.seekp(0);
    pd << "PD++";

    pd.put(MAX_SUPPORTED_VERSION_NUMBER);
    pd.put(checksum);
    pd.put(deriv);
    pd.put(hash);
    pd.put(hashIters);
    pd.put(keyFile);
    pd.put(encryption);

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);

    if (iv.empty()) {
        Botan::AutoSeeded_RNG rng;

        iv = rng.random_vec(enc->default_nonce_length());
    }

    pd << iv.data();
    pd << name;
    qDebug() << iv;
    pd.put(0);

    pd << desc;
    pd.put(0);

    std::string ptr = getPw(password);

    Botan::secure_vector<uint8_t> vPassword = toVec(ptr);

    enc->set_key(vPassword);

    Botan::secure_vector<uint8_t> pt(stList.data(), stList.data() + stList.length());

    std::unique_ptr<Botan::Compression_Algorithm> ptComp = Botan::Compression_Algorithm::create("gzip");

    ptComp->start();
    ptComp->finish(pt);

    enc->start(iv);
    enc->finish(pt);

    if (keyFile) {
        std::ifstream key(keyFilePath, std::ios::binary);
        Botan::secure_vector<uint8_t> keyData = Botan::secure_vector<uint8_t>(std::istreambuf_iterator<char>{key}, {});
        std::string keyPw = getPw(toStr(keyData));

        std::unique_ptr<Botan::Cipher_Mode> keyEnc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);

        keyEnc->set_key(toVec(keyPw));
        keyEnc->start(iv);
        keyEnc->finish(pt);
    }

    std::string pts = toStr(pt);
    data = pt;

    pd << pts;

    pd.flush();
    pd.close();
}

int Database::verify(std::string mpass) {
    qDebug() << iv;
    Botan::secure_vector<uint8_t> vPtr = toVec(getPw(mpass)), pData = data;

    if (keyFile) {
        std::ifstream key(keyFilePath, std::ios::binary);
        Botan::secure_vector<uint8_t> keyData = Botan::secure_vector<uint8_t>(std::istreambuf_iterator<char>{key}, {});
        std::string keyPw = getPw(toStr(keyData));

        std::unique_ptr<Botan::Cipher_Mode> keyDec = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::DECRYPTION);

        keyDec->set_key(toVec(keyPw));

        keyDec->start(iv);

        try {
            keyDec->finish(pData);
        } catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
            return 3;
        }
    }

    std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::DECRYPTION);

    decr->set_key(vPtr);
    decr->start(iv);

    try {
        decr->finish(pData);

        std::unique_ptr<Botan::Decompression_Algorithm> dataDe = Botan::Decompression_Algorithm::create("gzip");
        dataDe->start();
        dataDe->finish(pData);

        this->stList = toStr(pData);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }
    return true;
}

std::string Database::decrypt(std::string txt, std::string password) {
    std::string mpass;

    if (password == "") {
        QDialog *passDi = new QDialog;
        passDi->setWindowTitle(QWidget::tr("Enter your password"));

        QGridLayout *layout = new QGridLayout;

        QLabel *passLabel = new QLabel(QWidget::tr(std::string("Please enter your master password" + txt + ".").c_str()));

        QLineEdit *passEdit = new QLineEdit;
        passEdit->setEchoMode(QLineEdit::Password);
        passEdit->setCursorPosition(0);

        QLineEdit *keyEdit = new QLineEdit;
        QPushButton *getKf = new QPushButton(QWidget::tr("Open"));
        QDialogButtonBox *keyBox = new QDialogButtonBox;

        QLabel *keyLabel = new QLabel(QWidget::tr("Key File:"));

        if (keyFile) {
            QWidget::connect(getKf, &QPushButton::clicked, [keyEdit, this]() mutable {
                keyFilePath = getKeyFile();
                if (keyFilePath != "") {
                    keyFile = true;
                }
                keyEdit->setText(QString::fromStdString(keyFilePath));
            });

            keyBox->addButton(getKf, QDialogButtonBox::ActionRole);
        }

        QDialogButtonBox *passButtons = new QDialogButtonBox(QDialogButtonBox::Ok);
        QLabel *errLabel = new QLabel;
        errLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);
        errLabel->setLineWidth(2);

        errLabel->setMargin(5);

        QPalette palette;

        QColor lColor;
        lColor.setNamedColor("#DA4453");
        palette.setColor(QPalette::Light, lColor);

        QColor dColor;
        dColor.setNamedColor("#DA4453");
        palette.setColor(QPalette::Dark, dColor);

        QColor tColor;
        tColor.setNamedColor("#C4DA4453");
        palette.setColor(QPalette::Window, tColor);

        palette.setColor(QPalette::Text, Qt::white);

        QWidget::connect(passButtons->button(QDialogButtonBox::Ok), &QPushButton::clicked, [passEdit, passLabel, passDi, passButtons, errLabel, layout, palette, keyEdit, this]() mutable -> void {
            std::string pw = passEdit->text().toStdString();

            passDi->setCursor(QCursor(Qt::WaitCursor));

            QPalette textPal;
            textPal.setColor(QPalette::WindowText, Qt::darkGray);
            textPal.setColor(QPalette::ButtonText, Qt::darkGray);

            passLabel->setPalette(textPal);
            passButtons->setPalette(textPal);

            errLabel->setPalette(textPal);

            passDi->repaint();

            if (keyFile) {
                keyFilePath = keyEdit->text().toStdString();
            }

            if (pw == "") {
                return passDi->reject();
            }

            int ok = verify(pw);
            if (ok == 1) {
                return passDi->accept();
            }

            if (ok == 3) {
                errLabel->setText(QWidget::tr("Key File is invalid."));
            } else {
                errLabel->setText(QWidget::tr("Password is incorrect.\nIf this problem continues, the database may be corrupt."));
            }

            if (keyFile) {
                layout->addWidget(errLabel, 4, 0);
            } else {
                layout->addWidget(errLabel, 2, 0);
            }

            errLabel->setPalette(palette);

            passLabel->setPalette(QPalette());
            passButtons->setPalette(QPalette());

            passDi->unsetCursor();
        });

        layout->addWidget(passLabel);
        layout->addWidget(passEdit, 1, 0);
        if (keyFile) {
            layout->addWidget(keyLabel, 2, 0);
            layout->addWidget(keyEdit, 3, 0);
            layout->addWidget(keyBox, 3, 1);
            layout->addWidget(passButtons, 5, 0);
        } else {
            layout->addWidget(passButtons, 3, 0);
        }

        passDi->setLayout(layout);

        int ret = passDi->exec();

        if (ret == QDialog::Rejected) {
            return "";
        }

        keyFilePath = keyEdit->text().toStdString();
        kfp = keyFilePath;

        return passEdit->text().toStdString();
    } else {
        verify(password);
        return password;
    }
}

bool Database::save(std::string password) {
    std::string pw = decrypt(" to save");
    if (pw == "") {
        return false;
    }

    std::string mpass;
    if (pw != "") {
        mpass = pw;
    } else {
        mpass = password;
    }
    stList = saveSt();
    encrypt(mpass);

    modified = false;
    return true;
}

bool Database::convert() {
    std::ifstream pd(path, std::ios_base::binary | std::ios_base::out);
    std::string iv;
    std::getline(pd, iv);

    std::vector<uint8_t> ivd;
    try {
        ivd = Botan::hex_decode(iv);
    } catch (...) {
        return false;
    }
    std::string r(std::istreambuf_iterator<char>{pd}, {});

    if (ivd.size() != 12) {
        return false;
    }

    showMessage("This database may be for an older version of passman++. I will try to convert it now.");

    Botan::secure_vector<uint8_t> vData;
    std::string password;

    while (true) {
        vData = toVec(r);
        QString pass = QInputDialog::getText(nullptr, QWidget::tr("Enter your password"), QWidget::tr(std::string("Please enter your master password to convert the database.").c_str()), QLineEdit::Password);
        password = pass.toStdString();

        Botan::secure_vector<uint8_t> mptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();

        ph->derive_key(mptr.data(), mptr.size(), password.c_str(), password.size(), ivd.data(), ivd.size());

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create("AES-256/GCM", Botan::DECRYPTION);

        decr->set_key(mptr);
        decr->start(ivd);

        try {
            decr->finish(vData);
            break;
        }  catch (...) {
            displayErr("Wrong password, try again.");
        }
    }

    std::string name = QString(basename(path.c_str())).split(".")[0].toStdString();
    std::string rdata = toStr(vData);

    this->checksum = 1;
    this->deriv = 1;
    this->hash = 1;
    this->hashIters = 8;
    this->keyFile = false;
    this->encryption = 1;
    this->iv = Botan::secure_vector<uint8_t>(ivd.begin(), ivd.end());
    this->name = name;
    this->desc = "Converted from old database format.";
    this->stList = rdata;
    execAll(rdata);
    this->stList = saveSt();
    db.exec("DROP TABLE data");

    encrypt(password);
    pd.close();
    return true;
}

bool Database::showErr(std::string msg) {
    displayErr("Error: database file is corrupt or invalid.\nDetails: " + msg);
    return {};
}

bool Database::parse() {
    std::ifstream pd(path, std::ios_base::binary);

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);
    int ivLen = enc->default_nonce_length();

    char readData[ivLen];
    pd.read(readData, 4);
    if (std::string(readData, 4) != "PD++") {
        bool conv = convert();
        if (!conv) {
            return showErr("Invalid magic number \"" + std::string(readData) + "\".");
        }
        return true;
    }

    pd >> version;
    if (version > MAX_SUPPORTED_VERSION_NUMBER) {
        return showErr("Invalid version number.");
    }

    pd >> checksum;
    if (checksum >= checksumMatch.size()){
        return showErr("Invalid checksum option.");
    }

    pd >> deriv;
    if (deriv >= derivMatch.size()){
        return showErr("Invalid derivation option.");
    }

    pd >> hash;
    if (hash >= hashMatch.size()){
        return showErr("Invalid hash option.");
    }

    pd >> hashIters;
    qDebug() << hashIters;
    pd >> keyFile;

    pd >> encryption;
    if (encryption >= encryptionMatch.size()){
        return showErr("Invalid encryption option.");
    }

    QFile f(QString::fromStdString(path));
    f.open(QIODevice::ReadOnly);
    QDataStream q(&f);
    f.seek(11);
    q.readRawData(readData, ivLen);

    //pd.seekg(11);
    //pd.seekg(9).read(readData, ivLen);
    //iv = toVec(ivc, ivLen);
    iv = Botan::secure_vector<uint8_t>(readData, readData + ivLen);

    std::getline(pd, name, '\0');
    std::getline(pd, desc, '\0');

    data = Botan::secure_vector<uint8_t>(std::istreambuf_iterator<char>{pd}, {});

    pd.close();
    return true;
}

bool Database::config(bool create) {
    if (!create) {
        parse();
    }

    QDialog *di = new QDialog;
    di->setWindowTitle("Database Options");

    QFormLayout *layout = new QFormLayout;
    QMenuBar *bar = new QMenuBar;
    QMenu *help = bar->addMenu(QWidget::tr("Help"));
    help->addAction(QWidget::tr("Choosing Options"), []{
        QDesktopServices::openUrl(QUrl(choosingUrl));
    });

    auto comboBox = [layout, create](std::vector<const char *> vec, const char *label, int val) -> QComboBox* {
        QComboBox *box = new QComboBox;
        QStringList list = QStringList(vec.begin(), vec.end());
        box->addItems(list);
        box->setCurrentIndex(create ? 0 : val - 1);
        layout->addRow(QWidget::tr(label), box);
        return box;
    };

    QLineEdit *pass = new QLineEdit;
    pass->setPlaceholderText(QWidget::tr("Password"));
    pass->setEchoMode(QLineEdit::Password);
    layout->addRow(QWidget::tr("Password:"), pass);

    auto lineEdit = [layout](const char *text, std::string defText, const char *label) -> QLineEdit* {
        QLineEdit *le = new QLineEdit;
        le->setPlaceholderText(QWidget::tr(text));
        le->setText(QString::fromStdString(defText));

        layout->addRow(QWidget::tr(label), le);
        return le;
    };

    QLineEdit *nameEdit = lineEdit("Name", name, "Name:");
    QLineEdit *descEdit = lineEdit("Description", desc, "Description:");

    QComboBox *checksumBox = comboBox(checksumMatch, "Checksum Function:", checksum);
    QComboBox *derivBox = comboBox(derivMatch, "Key Derivation Function:", deriv);
    QComboBox *hashBox = comboBox(hashMatch, "Password Hashing Function:", hash);
    QComboBox *encryptionBox = comboBox(encryptionMatch, "Data Encryption Function:", encryption);

    int iterVal = create ? 8 : hashIters;

    QSpinBox *hashIterBox = new QSpinBox;
    hashIterBox->setRange(8, 40);
    hashIterBox->setSingleStep(1);
    hashIterBox->setValue(iterVal);

    layout->addRow(QWidget::tr("Password Hashing Iterations:"), hashIterBox);

    QLineEdit *keyEdit = new QLineEdit;
    keyEdit->setText(QString::fromStdString(kfp));

    QPushButton *newKf = new QPushButton(QWidget::tr("New"));
    QWidget::connect(newKf, &QPushButton::clicked, [keyEdit] {
        keyEdit->setText(QString::fromStdString(newKeyFile()));
    });

    QPushButton *getKf = new QPushButton(QWidget::tr("Open"));
    QWidget::connect(getKf, &QPushButton::clicked, [keyEdit] {
        keyEdit->setText(QString::fromStdString(getKeyFile()));
    });

    QDialogButtonBox *keyBox = new QDialogButtonBox;
    keyBox->addButton(newKf, QDialogButtonBox::ActionRole);
    keyBox->addButton(getKf, QDialogButtonBox::ActionRole);

    layout->addRow(QWidget::tr("Key File:"), keyEdit);
    layout->addWidget(keyBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(di);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QWidget::connect(buttonBox, &QDialogButtonBox::accepted, [pass, di, create]() mutable {
        std::string pw = pass->text().toStdString();
        if (create && pw == "") {
            displayErr("Password must be provided.");
        } else {
            if (create && pw.length() < 8) {
                std::cerr << "Warning: your password is less than 8 characters long. Consider making it longer." << std::endl;
            }
            di->accept();
        }
    });

    QWidget::connect(buttonBox, &QDialogButtonBox::rejected, di, &QDialog::reject);

    layout->addWidget(buttonBox);
    layout->setMenuBar(bar);

    di->setLayout(layout);
    pass->setCursorPosition(0);
    int ret = di->exec();

    if (ret == QDialog::Rejected) {
        return false;
    }

    if (create) {
        stList = getCreate("default", {"name", "email", "url", "notes", "password"}, {QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString)}, {"default", "default@createexample.com", "example.com", "This is a default, example entry.", "PASSWORDHERE"}).toStdString();
        execAll(stList);
    }

    std::string pw = pass->text().toStdString();
    std::string kfPath = keyEdit->text().toStdString();
    bool kf = !kfPath.empty();

    if (!std::experimental::filesystem::exists(kfPath)) {
        genKey(kfPath);
    }

    if (!create) {
        std::string dec = decrypt(" to save your new configuration");
        if (pw.empty()) {
            pw = dec;
        }
        stList = saveSt();
    }

    checksum = checksumBox->currentIndex();
    deriv = derivBox->currentIndex();
    hash = hashBox->currentIndex();
    hashIters = hashIterBox->value();
    encryption = encryptionBox->currentIndex();
    name = nameEdit->text().toStdString();
    desc = descEdit->text().toStdString();

    if (kf) {
        keyFilePath = kfPath;
    }
    keyFile = kf;

    if (name == "") {
        name = "None";
    }

    if (desc == "") {
        desc = "None";
    }

    encrypt(pw);

    return true;
}

bool Database::open() {
    if (std::experimental::filesystem::exists(path)) {
        if (!parse()) {
            return false;
        }
        if (stList == "") {
            try {
                std::string p = decrypt(" to login");
                if (p == "") {
                    return false;
                }
            } catch (std::exception& e) {
                displayErr(e.what());
                return false;
            }
        }
        std::string line;
        std::istringstream iss(stList);
        while (std::getline(iss, line)) {
            QSqlQuery q(db);
            bool ok = q.exec(QString::fromStdString(line));
            if (!ok) {
                qDebug() << "Warning: Error during database initialization:" << q.lastError();
            }
        }
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

int Database::backup() {
    QString fileName = QFileDialog::getSaveFileName(nullptr, QWidget::tr("Backup Location"), "", QWidget::tr(fileExt));
    if (fileName.isEmpty()) {
        return 3;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return 17;
    }
    try {
        path = fileName.toStdString();
        save();
    } catch (std::exception& e) {
        displayErr(e.what());
    }
    return true;
}
