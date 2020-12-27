#include <QComboBox>
#include <QSpinBox>
#include <QUrl>
#include <QDesktopServices>
#include <QInputDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QFormLayout>
#include <QPushButton>
#include <QMenuBar>
#include <QHeaderView>

#include "util/generators.h"
#include "entry.h"

Database::Database() {}

void showMessage(const QString &msg) {
    QMessageBox box;
    box.setText(msg);
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

std::string getCS(uint8_t cs, uint8_t encr) {
    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encr), Botan::ENCRYPTION);
    std::string checksumChoice(checksumMatch[cs]);

    if (checksumChoice != "SHA-512") {
        checksumChoice += "(" + std::to_string(enc->maximum_keylength() * 8) + ")";
    }
    return checksumChoice;
}

void Database::get() {
    entries = {};
    for (QSqlQuery q : selectAll()) {
        while (q.next()) {
            QList<Field *> fields;

            for (int i = 0; i < q.record().count(); ++i) {
                QVariant val = q.record().value(i);
                QString vName = q.record().fieldName(i);
                QMetaType::Type id = (QMetaType::Type)q.record().field(i).metaType().id();

                Field *f = new Field(vName, val, id);
                fields.push_back(f);
            }

            Entry *entry = new Entry(fields, this);
            entries.push_back(entry);
        }
    }
}

int Database::add(QTableWidget *table) {
    Entry *entry = new Entry({}, this);
    entry->setDefaults();
    entries.push_back(entry);

    int ret = entry->edit(nullptr, table);
    return ret;
}

int Database::edit() {
    QDialog *dialog = new QDialog;

    QDialogButtonBox *ok = new QDialogButtonBox(dialog);
    ok->setStandardButtons(QDialogButtonBox::Ok);
    QObject::connect(ok->button(QDialogButtonBox::Ok), &QPushButton::clicked, dialog, &QDialog::accept);

    QGridLayout *layout = new QGridLayout(dialog);

    QTableWidget *table = new QTableWidget(dialog);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSortingEnabled(true);

    QStringList labels{"Name", "Email", "URL"};
    table->setColumnCount(labels.length());
    table->setHorizontalHeaderLabels(labels);

    auto getNamed = [this](QTableWidget *table) -> Entry *{
        QString name = table->item(table->currentItem()->row(), 0)->text();

        return entryNamed(name);
    };

    QObject::connect(table, &QTableWidget::itemDoubleClicked, [getNamed](QTableWidgetItem *item) {
        getNamed(item->tableWidget())->edit(item);
    });

    QAction *addButton = this->addButton(QIcon::fromTheme(tr("list-add")), "Creates a new entry in the database.", QKeySequence(tr("Ctrl+N")), [table, this]{
        add(table);
    });

    QAction *delButton = this->addButton(QIcon::fromTheme(tr("edit-delete")), "Deletes the currently selected entry.", QKeySequence::Delete, [table, getNamed]{
        getNamed(table)->del(table->currentItem());
    });

    QAction *editButton = this->addButton(QIcon::fromTheme(tr("document-edit")), "Edit or view all the information of the current entry.", QKeySequence(tr("Ctrl+E")), [table, getNamed]{
        getNamed(table)->edit(table->currentItem());
    });

    QMenuBar *bar = new QMenuBar;
    QMenu *menu = bar->addMenu(tr("Edit"));
    menu->addActions(QList<QAction *>{addButton, delButton, editButton});

    layout->addWidget(table);
    layout->setMenuBar(bar);

    layout->addWidget(ok);

    dialog->setLayout(layout);
    dialog->setWindowTitle(tr("Select an entry"));

    dialog->resize(800, 450);
    redrawTable(table, this);

    dialog->exec();

    return true;
}

bool Database::saveSt(bool exec) {
    QString execSt = "";

    for (Entry *entry : entries) {
        execSt += entry->getCreate();
    }

    if (exec) {
        QSqlQuery delQuery(db);
        delQuery.exec("SELECT 'DROP TABLE \"' || name || '\"' FROM sqlite_master WHERE type = 'table'");
        QString delSt;

        while (delQuery.next()) {
            delSt += delQuery.value(0).toString() + "\n";
        }

        delQuery.finish();
        execAll(delSt);

        execAll(execSt);
    }
    this->stList = execSt;
    return true;
}

secvec Database::getPw(QString password) {
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

        if (verbose) {
            qDebug() << hashIters << QString::fromStdString(hashChoice) << QString::fromStdString(checksumChoice) << iv << password << Qt::endl;
        }

        secvec ptr(512);
        pHash->derive_key(ptr.data(), ptr.size(), password.toStdString().data(), password.size(), iv.data(), ivLen);
        password = toStr(ptr);
        if(verbose) {
            qDebug() << "After Hashing:" << QString::fromStdString(Botan::hex_encode(ptr)) << Qt::endl;
        }
    }

    secvec ptr(32);
    std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(" + checksumChoice + ")")->default_params();

    ph->derive_key(ptr.data(), ptr.size(), password.toStdString().data(), password.size(), iv.data(), ivLen);
    if (verbose) {
        qDebug() << "After Derivation:" << QString::fromStdString(Botan::hex_encode(ptr)) << Qt::endl;
    }

    return ptr;
}

void Database::encrypt(const QString &password) {
    std::ofstream pd(path.toStdString(), std::fstream::binary | std::fstream::trunc);
    pd.seekp(0);

    pd << "PD++";

    pd.put(MAX_SUPPORTED_VERSION_NUMBER);
    pd.put(checksum);
    pd.put(hash);
    pd.put(hashIters);
    pd.put(keyFile);
    pd.put(encryption);

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);

    if (iv.empty()) {
        Botan::AutoSeeded_RNG rng;
        ivLen = enc->default_nonce_length();
        iv = rng.random_vec(ivLen);
    }

    pd << iv.data();

    pd << name.toStdString();
    pd.put(10);

    pd << desc.toStdString();
    pd.put(10);

    secvec vPassword = getPw(password);

    enc->set_key(vPassword);
    if (verbose) {
        qDebug() << "STList before saveSt:" << stList.toStdString().data();
    }

    saveSt();

    if (verbose) {
        qDebug() << "STList after saveSt:" << stList.toStdString().data();
    }

    std::string stl = stList.toStdString();
    secvec pt(stl.data(), stl.data() + stl.length());

    std::unique_ptr<Botan::Compression_Algorithm> ptComp = Botan::Compression_Algorithm::create("gzip");

    ptComp->start();
    ptComp->finish(pt);

    enc->start(iv);
    enc->finish(pt);

    if (keyFile) {
        QFile kf(keyFilePath);
        kf.open(QIODevice::ReadOnly);
        QTextStream key(&kf);
        QString keyPw = key.readAll();

        std::unique_ptr<Botan::Cipher_Mode> keyEnc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);

        keyEnc->set_key(toVec(keyPw));
        keyEnc->start(iv);
        keyEnc->finish(pt);
    }

    data = pt;
    if (verbose) {
        qDebug() << "Data (Encryption):" << QString::fromStdString(Botan::hex_encode(pt));
    }
    std::string pts = toStdStr(pt);

    pd << pts;

    pd.flush();
    pd.close();
}

int Database::verify(const QString &mpass) {
    secvec vPtr = getPw(mpass), pData = data;

    if (keyFile) {
        QFile kf(keyFilePath);
        kf.open(QIODevice::ReadOnly);
        QTextStream key(&kf);
        QString keyPw = key.readAll();

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
        if (verbose) {
            qDebug() << "Data (Verification):" << Botan::hex_encode(pData).data();
        }
        decr->finish(pData);

        std::unique_ptr<Botan::Decompression_Algorithm> dataDe = Botan::Decompression_Algorithm::create("gzip");
        dataDe->start();
        dataDe->finish(pData);

        this->stList = toStr(pData);
        if (verbose) {
            qDebug() << "STList (verification):" << stList.toStdString().data();
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }
    return true;
}

QString Database::decrypt(const QString &txt, const QString &password) {
    if (password == "") {
        QDialog *passDi = new QDialog;
        passDi->setWindowTitle(QWidget::tr("Enter your password"));

        QGridLayout *layout = new QGridLayout;

        QLabel *passLabel = new QLabel("Please enter your master password" + txt + ".");

        QLineEdit *passEdit = new QLineEdit;
        passEdit->setEchoMode(QLineEdit::Password);
        passEdit->setCursorPosition(0);

        QLineEdit *keyEdit = new QLineEdit;
        QLabel *keyLabel = new QLabel(QWidget::tr("Key File:"));
        QPushButton *getKf = new QPushButton(QWidget::tr("Open"));
        QDialogButtonBox *keyBox = new QDialogButtonBox;

        if (keyFile) {
            QWidget::connect(getKf, &QPushButton::clicked, [keyEdit, this]() mutable {
                keyFilePath = getKeyFile();
                if (keyFilePath != "") {
                    keyFile = true;
                }
                keyEdit->setText(keyFilePath);
            });

            keyBox->addButton(getKf, QDialogButtonBox::ActionRole);
        } else {
            delete keyEdit;
            delete keyLabel;
            delete getKf;
            delete keyBox;
        }

        QDialogButtonBox *passButtons = new QDialogButtonBox(QDialogButtonBox::Ok);
        QLabel *errLabel = new QLabel;
        errLabel->setFrameStyle(QFrame::Panel | QFrame::Raised);
        errLabel->setLineWidth(2);

        errLabel->setMargin(5);

        QPalette palette;

        QColor _cl = QColor(218, 68, 83);

        palette.setColor(QPalette::Light, _cl);
        palette.setColor(QPalette::Dark, _cl);
        palette.setColor(QPalette::Window, QColor(218, 68, 83, 196));
        palette.setColor(QPalette::Text, Qt::white);

        QWidget::connect(passButtons->button(QDialogButtonBox::Ok), &QPushButton::clicked, [passEdit, passLabel, passDi, passButtons, errLabel, layout, palette, keyEdit, this]() mutable -> void {
            QString pw = passEdit->text();

            passDi->setCursor(QCursor(Qt::WaitCursor));

            QPalette textPal;
            textPal.setColor(QPalette::WindowText, Qt::darkGray);
            textPal.setColor(QPalette::ButtonText, Qt::darkGray);

            passLabel->setPalette(textPal);
            passButtons->setPalette(textPal);

            errLabel->setPalette(textPal);

            passDi->repaint();

            if (keyFile) {
                keyFilePath = keyEdit->text();
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

        if (keyFile) {
            keyFilePath = keyEdit->text();
        }

        return passEdit->text();
    } else {
        verify(password);
        return password;
    }
}

bool Database::save(const QString &password) {
    QString pw = decrypt(" to save");
    if (pw.isEmpty()) {
        return false;
    }

    QString mpass;
    if (pw != "") {
        mpass = pw;
    } else {
        mpass = password;
    }

    encrypt(mpass);

    modified = false;
    return true;
}

bool Database::convert() {
    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QTextStream pd(&f);
    QString iv = pd.readLine();

    std::vector<uint8_t> ivd;
    try {
        ivd = Botan::hex_decode(iv.toStdString());
    } catch (...) {
        return false;
    }

    if (ivd.size() != 12) {
        return false;
    }
    QString r = pd.readAll();

    showMessage("This database may be for an older version of passman++. I will try to convert it now.");

    secvec vData;
    QString password;

    while (true) {
        vData = toVec(r);
        password = QInputDialog::getText(nullptr, QWidget::tr("Enter your password"), QWidget::tr("Please enter your master password to convert the database."), QLineEdit::Password);

        secvec mptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();

        ph->derive_key(mptr.data(), mptr.size(), password.toStdString().data(), password.size(), ivd.data(), ivd.size());

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
    QString name = QString(basename(path.toStdString().data())).split(".")[0];
    QString rdata = toStr(vData);

    this->checksum = 0;
    this->hash = 0;
    this->hashIters = 8;
    this->keyFile = false;
    this->encryption = 0;
    this->iv = secvec(ivd.begin(), ivd.end());
    this->name = name;
    this->desc = "Converted from old database format.";
    this->stList = rdata;
    execAll(rdata);
    get();
    saveSt();
    db.exec("DROP TABLE data");

    encrypt(password);
    f.close();
    return true;
}

bool Database::showErr(QString msg) {
    displayErr("Error: database file is corrupt or invalid.\nDetails: " + msg);
    return {};
}

bool Database::parse() {
    char readData[4];

    QFile f(path);
    f.open(QIODevice::ReadOnly);
    QDataStream q(&f);

    q.readRawData(readData, 4);

    if (std::string(readData, 4) != "PD++") {
        bool conv = convert();
        if (!conv) {
            return showErr("Invalid magic number \"" + QString(readData) + "\".");
        }
        return true;
    }

    q >> version;
    if (version > MAX_SUPPORTED_VERSION_NUMBER) {
        return showErr("Invalid version number.");
    }

    q >> checksum;
    if (checksum >= checksumMatch.size()){
        return showErr("Invalid checksum option.");
    }

    if (version < 6) {
        uint8_t n;
        q >> n;
    }

    q >> hash;
    if (hash >= hashMatch.size()){
        return showErr("Invalid hash option.");
    }

    q >> hashIters;
    q >> keyFile;

    q >> encryption;
    if (encryption >= encryptionMatch.size()){
        return showErr("Invalid encryption option.");
    }

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);
    ivLen = enc->default_nonce_length();

    char ivc[ivLen];
    q.readRawData(ivc, ivLen);
    iv = toVec(ivc, ivLen);

    char namec[255], descc[255];
    f.readLine(namec, 255);
    f.readLine(descc, 255);

    desc = QString(namec).trimmed();
    name = QString(descc).trimmed();

    int available = f.bytesAvailable();
    char datac[available];
    q.readRawData(datac, available);

    data = toVec(datac, available);

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

    auto comboBox = [layout, create](QList<std::string> vec, const char *label, int val) -> QComboBox* {
        QComboBox *box = new QComboBox;

        QStringList list;
        for (int i = 0; i < vec.size(); ++i) {
            list.push_back(QString::fromStdString(vec[i]));
        }

        box->addItems(list);
        box->setCurrentIndex(create ? 0 : val);
        layout->addRow(QWidget::tr(label), box);
        return box;
    };

    QLineEdit *pass = new QLineEdit;
    pass->setPlaceholderText(QWidget::tr("Password"));
    pass->setEchoMode(QLineEdit::Password);
    layout->addRow(QWidget::tr("Password:"), pass);

    auto lineEdit = [layout](const char *text, QString defText, const char *label) -> QLineEdit* {
        QLineEdit *le = new QLineEdit;
        le->setPlaceholderText(QWidget::tr(text));
        le->setText(defText);

        layout->addRow(QWidget::tr(label), le);
        return le;
    };

    QLineEdit *nameEdit = lineEdit("Name", name, "Name:");
    QLineEdit *descEdit = lineEdit("Description", desc, "Description:");

    QComboBox *checksumBox = comboBox(checksumMatch, "Checksum Function:", checksum);
    QComboBox *hashBox = comboBox(hashMatch, "Password Hashing Function:", hash);
    QComboBox *encryptionBox = comboBox(encryptionMatch, "Data Encryption Function:", encryption);

    if (debug) {
        qDebug() << "Database params:" << checksum << hash << encryption;
    }

    int iterVal = create ? 8 : hashIters;

    QSpinBox *hashIterBox = new QSpinBox;
    hashIterBox->setRange(8, 40);
    hashIterBox->setSingleStep(1);
    hashIterBox->setValue(iterVal);

    layout->addRow(QWidget::tr("Password Hashing Iterations:"), hashIterBox);

    QLineEdit *keyEdit = new QLineEdit;
    keyEdit->setText(keyFilePath);

    QPushButton *newKf = new QPushButton(QWidget::tr("New"));
    QWidget::connect(newKf, &QPushButton::clicked, [keyEdit] {
        keyEdit->setText(newKeyFile());
    });

    QPushButton *getKf = new QPushButton(QWidget::tr("Open"));
    QWidget::connect(getKf, &QPushButton::clicked, [keyEdit] {
        keyEdit->setText(getKeyFile());
    });

    QDialogButtonBox *keyBox = new QDialogButtonBox;
    keyBox->addButton(newKf, QDialogButtonBox::ActionRole);
    keyBox->addButton(getKf, QDialogButtonBox::ActionRole);

    layout->addRow(QWidget::tr("Key File:"), keyEdit);
    layout->addWidget(keyBox);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(di);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QWidget::connect(buttonBox, &QDialogButtonBox::accepted, [pass, di, create]() mutable {
        QString pw = pass->text();
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

    pass->setFocus(Qt::MouseFocusReason);
    int ret = di->exec();

    if (ret == QDialog::Rejected) {
        return false;
    }

    if (create) {
        Entry *entry = new Entry({}, this);
        entry->setDefaults();
        for (Field *f : entry->getFields()) {
            f->setData("EXAMPLE");
        }
    }

    QString pw = pass->text();
    QString kfPath = keyEdit->text();
    bool kf = !kfPath.isEmpty();

    if (kf && !QFile::exists(kfPath)) {
        genKey(kfPath);
    }

    if (!create) {
        QString dec = decrypt(" to save your new configuration");
        if (dec.isEmpty()) {
            return false;
        }
        if (pw.isEmpty()) {
            pw = dec;
        }
    }

    checksum = checksumBox->currentIndex();
    hash = hashBox->currentIndex();
    hashIters = hashIterBox->value();
    encryption = encryptionBox->currentIndex();
    name = nameEdit->text();
    desc = descEdit->text();

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
    if (QFile::exists(path)) {
        if (!parse()) {
            return false;
        }

        if (stList == "") {
            try {
                QString p = decrypt(" to login", "");
                if (p.isEmpty()) {
                    return false;
                }
            } catch (std::exception& e) {
                displayErr(e.what());
                return false;
            }
        }

        for (const QString &line : stList.split('\n')) {
            if (line == "") {
                continue;
            }
            QSqlQuery q(db);
            bool ok = q.exec(line);
            if (!ok) {
                qDebug() << "Warning: Error during database initialization:" << q.lastError();
            }
        }
        get();
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

int Database::backup() {
    QString fileName = QFileDialog::getSaveFileName(nullptr, QWidget::tr("Backup Location"), "", fileExt);
    if (fileName.isEmpty()) {
        return 3;
    }
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return 17;
    }
    try {
        path = fileName;
        bool ok = save();
        if (!ok) {
            return false;
        }
    } catch (std::exception& e) {
        displayErr(e.what());
    }
    return true;
}

Entry *Database::entryNamed(QString &name) {
    for (Entry *e : entries) {
        if (e->getName() == name) {
            return e;
        }
    }

    return nullptr;
}

void Database::addEntry(Entry *entry) {
    entries.push_back(entry);
}

bool Database::removeEntry(Entry *entry) {
    return entries.removeOne(entry);
}

int Database::entryLength() {
    return entries.length();
}

QList<Entry *> &Database::getEntries() {
    return entries;
}

void Database::setEntries(QList<Entry *> entries) {
    this->entries = entries;
}

template <typename Func>
QAction *Database::addButton(QIcon icon, const char *whatsThis, QKeySequence shortcut, Func func) {
    QAction *action = new QAction(icon, "");
    action->setWhatsThis(tr(whatsThis));
    action->setShortcut(shortcut);
    QObject::connect(action, &QAction::triggered, func);
    return action;
}
