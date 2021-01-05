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
#include <QToolButton>
#include <QApplication>
#include <QClipboard>
#include <QTimer>
#include <QCheckBox>
#include <QSizePolicy>

#include "util/generators.h"
#include "entry.h"

Database::Database() {}

void showMessage(const QString &msg) {
    QMessageBox box;

    box.setText(msg);
    box.setStandardButtons(QMessageBox::Ok);
    box.setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);

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
                QString val = q.record().value(i).toString().replace(" || char(10) || ", "\n");
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

    QStringList labels{"Name", "Email", "URL", "Notes"};
    table->setColumnCount(labels.length());
    table->setHorizontalHeaderLabels(labels);

    auto getNamed = [this](QTableWidget *table) -> Entry *{
        QTableWidgetItem *item = table->item(table->currentRow(), 0);

        if (item == nullptr) {
            return nullptr;
        }

        QString eName = item->text();

        return entryNamed(eName);
    };

    QObject::connect(table, &QTableWidget::itemDoubleClicked, [getNamed](QTableWidgetItem *item) {
        getNamed(item->tableWidget())->edit(item);
    });

    QMenuBar *bar = new QMenuBar;

    QAction *saveButton = this->addButton("Save", "document-save", "Save the database as is.", QKeySequence(tr("Ctrl+S")), [this] {
        this->save();
    });

    QAction *saveAsButton = this->addButton("Save as...", "document-save-as", "Save the database to a different location.", QKeySequence(tr("Ctrl+Shift+S")), [this] {
        int br = this->backup();
        if (br == 3) {
            displayErr("Invalid backup location.");
        } else if (br == 17) {
            displayErr("Improper permissions for file. Please select a location where the current user has write permissions.");
        }
    });

    QMenu *fileMenu = bar->addMenu(tr("File"));
    fileMenu->addActions(QList<QAction *>{saveButton, saveAsButton});
    fileMenu->addSeparator();

    QAction *configButton = this->addButton("Edit Database", "document-edit", "Edit database options.", QKeySequence(tr("Ctrl+Shift+E")), [this] {
        this->config(false);
    });

    fileMenu->addAction(configButton);

    QAction *addButton = this->addButton("New", "list-add", "Creates a new entry in the database.", QKeySequence(tr("Ctrl+N")), [table, this]{
        add(table);
    });

    QAction *delButton = this->addButton("Delete", "edit-delete", "Deletes the currently selected entry.", QKeySequence::Delete, [table, getNamed]{
        Entry *named = getNamed(table);
        if (named == nullptr) {
            return;
        }
        named->del(table->currentItem());
    });

    QAction *editButton = this->addButton("Edit", "document-edit", "Edit or view all the information of the current entry.", QKeySequence(tr("Ctrl+E")), [table, getNamed]{
        Entry *named = getNamed(table);
        if (named == nullptr) {
            return;
        }
        named->edit(table->currentItem());
    });

    QMenu *entryMenu = bar->addMenu(tr("Entry"));
    entryMenu->addActions(QList<QAction *>{addButton, delButton, editButton});

    entryMenu->addSeparator();

    QAction *copyPasswordButton = this->addButton("Copy Password", "edit-copy", "Copy this entry's password. Clipboard will be cleared after a configurable time.", QKeySequence(tr("Ctrl+C")), [table, this] {
        QClipboard *clip = qApp->clipboard();

        QTableWidgetItem *item = table->item(table->currentRow(), 0);

        if (item == nullptr) {
            return;
        }

        QString eName = item->text();
        Entry *en = entryNamed(eName);
        clip->setText(en->fieldNamed("Password")->dataStr());

        QTimer::singleShot(clearSecs * 1000, [clip] {
            clip->setText("");
        });
    });

    entryMenu->addAction(copyPasswordButton);

    QAction *aboutButton = this->addButton("About", "help-about", "About passman++", QKeySequence(tr("Ctrl+H")), [] {
        showMessage(info);
    });

    QAction *tipsButton = this->addButton("Tips", "help-hint", "Tips for good password management.", QKeySequence(), [] {
        QDesktopServices::openUrl(QUrl("https://github.com/binex-dsk/passmanpp/blob/main/tips.md"));
    });

    QMenu *aboutMenu = bar->addMenu(tr("About"));
    aboutMenu->addActions(QList<QAction *>{aboutButton, tipsButton});
    aboutMenu->addAction(tr("About Qt..."), qApp, &QApplication::aboutQt);

    layout->addWidget(table);
    layout->setMenuBar(bar);

    QFont font;
    font.setBold(true);

    QWidget *prevWidg = new QWidget;
    QGridLayout *preview = new QGridLayout(prevWidg);

    QLabel *nameValue = new QLabel(prevWidg);
    preview->addWidget(nameValue, 0, 2);

    QLabel *emailValue = new QLabel(prevWidg);
    preview->addWidget(emailValue, 1, 2);

    QLabel *urlValue = new QLabel(prevWidg);
    preview->addWidget(urlValue, 2, 2);

    QLabel *passValue = new QLabel(prevWidg);
    preview->addWidget(passValue, 3, 2);

    QLabel *nameLabel = new QLabel(tr("Name"), prevWidg);
    nameLabel->setFont(font);
    preview->addWidget(nameLabel, 0, 0);

    QLabel *emailLabel = new QLabel(tr("Email"), prevWidg);
    emailLabel->setFont(font);
    preview->addWidget(emailLabel, 1, 0);

    QLabel *urlLabel = new QLabel(tr("URL"), prevWidg);
    urlLabel->setFont(font);
    preview->addWidget(urlLabel, 2, 0);

    QLabel *passLabel = new QLabel(tr("Password"), prevWidg);
    passLabel->setFont(font);
    preview->addWidget(passLabel, 3, 0);

    layout->addWidget(ok, 2, 0);

    QObject::connect(table, &QTableWidget::itemSelectionChanged, [table, layout, prevWidg, nameValue, emailValue, urlValue, passValue, this] {
        bool anySelected = table->selectedItems().length() > 0;

        prevWidg->setVisible(anySelected);
        if (!anySelected) {
            return layout->removeWidget(prevWidg);
        }

        QString eName = table->item(table->currentRow(), 0)->text();

        Entry *selected = entryNamed(eName);
        nameValue->setText(selected->getName());
        emailValue->setText(selected->fieldNamed("Email")->dataStr());
        urlValue->setText(selected->fieldNamed("URL")->dataStr());

        QString passText;
        for (int i = 0; i < selected->fieldNamed("Password")->dataStr().length(); ++i) {
            passText += "●";
        }
        passValue->setText(passText);

        layout->addWidget(prevWidg, 1, 0);
    });

    QToolButton *passView = new QToolButton(prevWidg);
    passView->setIcon(QIcon::fromTheme("view-visible"));
    passView->setCheckable(true);

    QObject::connect(passView, &QToolButton::clicked, [table, passValue, this](bool checked) {
        QString eName = table->item(table->currentRow(), 0)->text();

        Entry *selected = entryNamed(eName);
        QString pass = selected->fieldNamed("Password")->dataStr();

        if (checked) {
            passValue->setText(pass);
        } else {
            QString passText;
            for (int i = 0; i < pass.length(); ++i) {
                passText += "●";
            }
            passValue->setText(passText);
        }
    });

    preview->addWidget(passView, 3, 1);

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
    this->stList = toVec(execSt);
    return true;
}

secvec Database::getPw(QString password) {
    std::string checksumChoice = getCS(checksum, encryption);
    std::string hashChoice = hashMatch[hash];

    if (hash < 3) {
        std::unique_ptr<Botan::PasswordHashFamily> pfHash = Botan::PasswordHashFamily::create(hashChoice);
        std::unique_ptr<Botan::PasswordHash> pHash;

        if (hash == 0) {
            pHash = pfHash->from_params(memoryUsage * 1000, hashIters, 1);
        } else if (hash == 2) {
            pHash = pfHash->from_params(32768, hashIters, 1);
        } else {
            pHash = pfHash->from_params(hashIters);
        }

        if (verbose) {
            qDebug() << hashIters << hashChoice.data() << checksumChoice.data() << iv << password << Qt::endl;
        }

        secvec ptr(512);
        pHash->derive_key(ptr.data(), ptr.size(), password.toStdString().data(), password.size(), iv.data(), ivLen);
        password = toStr(ptr);
        if(verbose) {
            qDebug() << "After Hashing:" << Botan::hex_encode(ptr).data() << Qt::endl;
        }
    }

    secvec ptr(32);
    std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(" + checksumChoice + ")")->default_params();

    ph->derive_key(ptr.data(), ptr.size(), password.toStdString().data(), password.size(), iv.data(), ivLen);
    if (verbose) {
        qDebug() << iv;
        qDebug() << "After Derivation:" << Botan::hex_encode(ptr).data() << Qt::endl;
    }

    return ptr;
}

void Database::encrypt() {
    std::ofstream pd(path.toStdString(), std::fstream::binary | std::fstream::trunc);
    pd.seekp(0);

    pd << "PD++";

    pd.put(MAX_SUPPORTED_VERSION_NUMBER);
    pd.put(checksum);
    pd.put(hash);

    if (hash != 3) {
        pd.put(hashIters);
    }

    pd.put(keyFile);
    pd.put(encryption);

    if (hash == 0) {
        pd.put(memoryUsage >> 8);
        pd.put(memoryUsage & 0xFF);
    }

    pd.put(clearSecs);
    pd.put(compress);

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);

    pd << iv.data();

    pd << name.toStdString();
    pd.put(10);

    pd << desc.toStdString();
    pd.put(10);

    enc->set_key(passw);
    if (verbose) {
        qDebug() << "STList before saveSt:" << toStr(stList);
    }

    saveSt();

    if (verbose) {
        qDebug() << "STList after saveSt:" << toStr(stList);
    }

    secvec pt = stList;

    if (compress) {
        std::unique_ptr<Botan::Compression_Algorithm> ptComp = Botan::Compression_Algorithm::create("gzip");

        ptComp->start();
        ptComp->finish(pt);
    }

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
        qDebug() << "Data (Encryption):" << Botan::hex_encode(pt).data();
    }

    for (unsigned long i = 0; i < pt.size(); ++i) {
        pd.put(pt[i]);
    }

    pd.flush();
    pd.close();
}

int Database::verify(const QString &mpass, bool convert) {
    if (convert) {
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
            displayErr("This is not a valid database file.");
            exit(EXIT_FAILURE);
        }
        QString r = pd.readAll();

        secvec vData = toVec(r);

        secvec mptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();

        ph->derive_key(mptr.data(), mptr.size(), mpass.toStdString().data(), mpass.size(), ivd.data(), ivd.size());

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create("AES-256/GCM", Botan::DECRYPTION);

        decr->set_key(mptr);
        decr->start(ivd);

        try {
            decr->finish(vData);
            this->passw = getPw(mpass);
        }  catch (...) {
            return false;
        }

        QString rdata = toStr(vData);

        this->iv = secvec(ivd.begin(), ivd.end());
        this->name = QString(basename(path.toStdString().data())).split(".")[0];;
        this->desc = "Converted from old database format.";
        this->stList = vData;
        execAll(rdata);

        QSqlQuery q(db);
        q.exec("SELECT tbl_name FROM sqlite_master WHERE type='table'");

        while (q.next()) {
            QString tblName = q.value(0).toString();
            db.exec("CREATE TABLE '" + tblName + "_COPY' (Name text, Email text, URL text, Notes blob, Password text)");
            db.exec("INSERT INTO '" + tblName + "_COPY' SELECT name, email, url, notes, password FROM '" + tblName + "'");
            db.exec("DROP TABLE '" + tblName + "'");
            db.exec("ALTER TABLE '" + tblName + "_COPY' RENAME TO '" + tblName + "'");
        }
        get();
        saveSt();
        db.exec("DROP TABLE data");

        encrypt();
        f.close();
        return true;
    }

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

        if (compress) {
            std::unique_ptr<Botan::Decompression_Algorithm> dataDe = Botan::Decompression_Algorithm::create("gzip");
            dataDe->start();
            dataDe->finish(pData);
        }

        this->stList = pData;
        this->passw = vPtr;
        if (verbose) {
            qDebug() << "STList (verification):" << toStr(stList);
        }
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 2;
    }
    return true;
}

QString Database::decrypt(const QString &txt, bool convert) {
    QDialog *passDi = new QDialog;
    passDi->setWindowTitle(tr("Enter your password"));

    QGridLayout *layout = new QGridLayout;

    QLabel *passLabel = new QLabel("Please enter your master password" + txt + ".");

    QLineEdit *passEdit = new QLineEdit;
    passEdit->setEchoMode(QLineEdit::Password);
    passEdit->setCursorPosition(0);

    QLineEdit *keyEdit = new QLineEdit;
    QLabel *keyLabel = new QLabel(tr("Key File:"));
    QPushButton *getKf = new QPushButton(tr("Open"));
    QDialogButtonBox *keyBox = new QDialogButtonBox;

    if (keyFile) {
        QObject::connect(getKf, &QPushButton::clicked, [keyEdit, this]() mutable {
            keyFilePath = getKeyFile();
            keyFile = keyFilePath != "";
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

    QObject::connect(passButtons->button(QDialogButtonBox::Ok), &QPushButton::clicked, [passEdit, passLabel, passDi, passButtons, errLabel, layout, palette, keyEdit, convert, this]() mutable {
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

        if (pw.isEmpty()) {
            return passDi->reject();
        }

        int ok = verify(pw, convert);
        if (ok == 1) {
            return passDi->accept();
        }

        if (ok == 3) {
            errLabel->setText(tr("Key File is invalid."));
        } else {
            errLabel->setText(tr("Password is incorrect.\nIf this problem continues, the database may be corrupt."));
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
}

bool Database::save() {
    encrypt();

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
        password = QInputDialog::getText(nullptr, tr("Enter your password"), tr("Please enter your master password to convert the database."), QLineEdit::Password);

        secvec mptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();

        ph->derive_key(mptr.data(), mptr.size(), password.toStdString().data(), password.size(), ivd.data(), ivd.size());

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create("AES-256/GCM", Botan::DECRYPTION);

        decr->set_key(mptr);
        decr->start(ivd);

        try {
            decr->finish(vData);
            this->passw = mptr;
            break;
        }  catch (...) {
            displayErr("Wrong password, try again.");
        }
    }

    QString rdata = toStr(vData);

    this->checksum = 0;
    this->hash = 0;
    this->hashIters = 8;
    this->keyFile = false;
    this->encryption = 0;
    this->iv = secvec(ivd.begin(), ivd.end());
    this->name = QString(basename(path.toStdString().data())).split(".")[0];;
    this->desc = "Converted from old database format.";
    this->stList = vData;
    execAll(rdata);
    get();
    saveSt();
    db.exec("DROP TABLE data");

    encrypt();
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
        q.skipRawData(1);
    }

    q >> hash;
    if (hash >= hashMatch.size()){
        return showErr("Invalid hash option.");
    }

    if (hash != 3) {
        q >> hashIters;
    }

    q >> keyFile;

    q >> encryption;
    if (encryption >= encryptionMatch.size()){
        return showErr("Invalid encryption option.");
    }

    if (version >= 7) {
        if (hash == 0) {
            q >> memoryUsage;
        }
        q >> clearSecs;
        q >> compress;
    }

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);
    ivLen = enc->default_nonce_length();

    char ivc[ivLen];
    q.readRawData(ivc, ivLen);
    iv = toVec(ivc, ivLen);

    QByteArray namec = f.readLine(), descc = f.readLine();

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

    QPalette diPalette;
    QColor _diC = QColor(54, 54, 56);
    diPalette.setColor(QPalette::Window, _diC);
    diPalette.setColor(QPalette::Base, _diC);

    di->setPalette(diPalette);

    QGridLayout *full = new QGridLayout(di);

    QMenuBar *bar = new QMenuBar;
    QMenu *help = bar->addMenu(tr("Help"));
    help->addAction(tr("Choosing Options"), []{
        QDesktopServices::openUrl(QUrl("https://github.com/binex-dsk/passmanpp/blob/main/Choosing%20Options.md"));
    });
    full->setMenuBar(bar);

    QFont bold;
    bold.setBold(true);

    QFont italic;
    italic.setItalic(true);

    QPalette sectPalette;
    QColor _window = QColor(62, 62, 66);
    QColor _border = QColor(86, 86, 90);

    sectPalette.setColor(QPalette::Light, _border);
    sectPalette.setColor(QPalette::Dark, _border);
    sectPalette.setColor(QPalette::Window, _window);

    QLabel *metaTitle = new QLabel(tr("General Info"));
    metaTitle->setFont(bold);
    full->addWidget(metaTitle);

    QFrame *metaWidget = new QFrame;
    metaWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    metaWidget->setLineWidth(2);

    metaWidget->setAutoFillBackground(true);

    metaWidget->setPalette(sectPalette);
    QFormLayout *metaLayout = new QFormLayout(metaWidget);

    QLabel *metaDesc = new QLabel(tr("Optional name and description for your database."));
    metaLayout->addRow(metaDesc);

    auto lineEdit = [metaLayout](const char *text, QString defText, const char *label) -> QLineEdit* {
        QLineEdit *le = new QLineEdit;

        le->setPlaceholderText(tr(text));
        le->setText(defText);

        metaLayout->addRow(tr(label), le);
        return le;
    };

    QLineEdit *nameEdit = lineEdit("Name", name, "  Database Name:");
    QLineEdit *descEdit = lineEdit("Description", desc, "  Description:");

    full->addWidget(metaWidget);

    QLabel *encTitle = new QLabel(tr("Security Settings"));
    encTitle->setFont(bold);
    full->addWidget(encTitle);

    QFrame *encWidget = new QFrame;
    encWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    encWidget->setLineWidth(2);

    encWidget->setAutoFillBackground(true);

    encWidget->setPalette(sectPalette);
    QFormLayout *encLayout = new QFormLayout(encWidget);

    QLabel *encDesc = new QLabel(tr("Adjust encryption, hashing, and checksum functions, as well as some additional parameters."));
    encLayout->addRow(encDesc);

    auto comboBox = [encLayout, create](QList<std::string> vec, const char *label, int val) -> QComboBox* {
        QComboBox *box = new QComboBox;

        QStringList list;
        for (int i = 0; i < vec.size(); ++i) {
            list.push_back(QString::fromStdString(vec[i]));
        }

        box->addItems(list);
        box->setCurrentIndex(create ? 0 : val);
        encLayout->addRow(tr(label), box);
        return box;
    };

    QComboBox *checksumBox = comboBox(checksumMatch, "  Checksum Function:", checksum);
    QComboBox *hashBox = comboBox(hashMatch, "  Password Hashing Function:  ", hash);
    QComboBox *encryptionBox = comboBox(encryptionMatch, "  Data Encryption Function:", encryption);

    if (debug) {
        qDebug() << "Database params:" << checksum << hash << encryption;
    }

    int iterVal = hashIters;

    QSpinBox *hashIterBox = new QSpinBox;
    hashIterBox->setRange(8, 40);
    hashIterBox->setSingleStep(1);
    hashIterBox->setValue(iterVal);
    hashIterBox->setToolTip(tr("How many times to hash the password."));

    encLayout->addRow(tr("  Password Hashing Iterations:"), hashIterBox);

    QSpinBox *memBox = new QSpinBox;
    memBox->setRange(1, 65535);
    memBox->setSingleStep(1);
    memBox->setSuffix(" MB");
    memBox->setValue(64);
    memBox->setToolTip(tr("How much memory, in MB, to use for password hashing."));

    encLayout->addRow(tr("  Memory Usage:"), memBox);

    auto calcMem = [hashIterBox, memBox, hashBox] {
        if (hashBox->currentIndex() == 2) {
            int val = hashIterBox->value() * 128 * 32768;
            double dispVal = std::round(val / 10000) / 100;
            memBox->setValue(dispVal);
        }
    };

    QObject::connect(hashIterBox, &QSpinBox::valueChanged, calcMem);

    calcMem();

    auto updateBoxes = [hashIterBox, memBox, encLayout, calcMem](int index) {
        bool hashVis = (index != 3);
        hashIterBox->setVisible(hashVis);
        encLayout->labelForField(hashIterBox)->setVisible(hashVis);

        bool memVis = (index == 0 || index == 2);
        memBox->setVisible(memVis);
        encLayout->labelForField(memBox)->setVisible(memVis);
        calcMem();

        bool memRO = (index == 2);
        memBox->setReadOnly(memRO);
    };

    QObject::connect(hashBox, &QComboBox::currentIndexChanged, updateBoxes);

    updateBoxes(hash);

    full->addWidget(encWidget);

    QLabel *passTitle = new QLabel(tr("Authentication Settings"));
    passTitle->setFont(bold);
    full->addWidget(passTitle);

    QFrame *passWidget = new QFrame;
    passWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    passWidget->setLineWidth(2);

    passWidget->setAutoFillBackground(true);

    passWidget->setPalette(sectPalette);
    QFormLayout *passLayout = new QFormLayout(passWidget);

    QLabel *passDesc = new QLabel(tr("Password and optional key file."));
    passLayout->addRow(passDesc);

    QLineEdit *pass = new QLineEdit;
    pass->setPlaceholderText(tr("Password"));
    pass->setEchoMode(QLineEdit::Password);
    passLayout->addRow(tr("  Password:"), pass);

    QLineEdit *keyEdit = new QLineEdit;
    keyEdit->setText(keyFilePath);

    QPushButton *newKf = new QPushButton(tr("New"));
    QObject::connect(newKf, &QPushButton::clicked, [keyEdit] {
        keyEdit->setText(newKeyFile());
    });

    QPushButton *getKf = new QPushButton(tr("Open"));
    QObject::connect(getKf, &QPushButton::clicked, [keyEdit] {
        keyEdit->setText(getKeyFile());
    });

    QDialogButtonBox *keyBox = new QDialogButtonBox;
    keyBox->addButton(newKf, QDialogButtonBox::ActionRole);
    keyBox->addButton(getKf, QDialogButtonBox::ActionRole);

    passLayout->addRow(tr("  Key File:"), keyEdit);
    passLayout->addRow(keyBox);
    passLayout->setAlignment(keyBox, Qt::AlignLeft);

    QLabel *keyWarn = new QLabel(tr("  Note: Do not select a file that may, at any point, change. This will make your database entirely inaccessible! It's recommended to create a new key file rather than use an existing one, unless it is a *.pkpp file."));
    keyWarn->setFont(italic);

    passLayout->addRow(keyWarn);

    full->addWidget(passWidget);

    QLabel *miscTitle = new QLabel(tr("Miscellaneous Settings"));
    miscTitle->setFont(bold);
    full->addWidget(miscTitle);

    QFrame *miscWidget = new QFrame;
    miscWidget->setFrameStyle(QFrame::Panel | QFrame::Raised);
    miscWidget->setLineWidth(2);

    miscWidget->setAutoFillBackground(true);

    miscWidget->setPalette(sectPalette);
    QFormLayout *miscLayout = new QFormLayout(miscWidget);

    QLabel *miscDesc = new QLabel(tr("Other, unrelated settings."));
    miscLayout->addRow(miscDesc);

    QCheckBox *compressBox = new QCheckBox;
    compressBox->setChecked(compress);

    miscLayout->addRow(tr("  Compression (recommended):"), compressBox);

    QSpinBox *clearBox = new QSpinBox;
    clearBox->setRange(1, 255);
    clearBox->setSingleStep(1);
    clearBox->setSuffix(" seconds");
    clearBox->setValue(clearSecs);
    clearBox->setToolTip(tr("How long to wait before clearing the clipboard when copying passwords"));

    miscLayout->addRow(tr("  Clipboard Clear Delay:"), clearBox);

    full->addWidget(miscWidget);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(di);
    buttonBox->setStandardButtons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

    QObject::connect(buttonBox, &QDialogButtonBox::accepted, [pass, di, create, encryptionBox, hashBox, checksumBox, memBox, hashIterBox, this]() mutable {
        QString pw = pass->text();
        bool paramsChanged = encryptionBox->currentIndex() != encryption || hashBox->currentIndex() != hash || checksumBox->currentIndex() != checksum || memBox->value() != memoryUsage || hashIterBox->value() != hashIters;

        if ((create || paramsChanged) && pw.isEmpty()) {
            displayErr("Password must be provided.");
        } else {
            if (create && pw.length() < 8) {
                std::cerr << "Warning: your password is less than 8 characters long. Consider making it longer." << std::endl;
            }
            if (paramsChanged) {
                di->setCursor(QCursor(Qt::WaitCursor));
                passw = getPw(pw);
            }
            di->accept();
        }
    });

    QObject::connect(buttonBox, &QDialogButtonBox::rejected, di, &QDialog::reject);

    QWidget *glw = new QWidget;
    glw->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QGridLayout *gl = new QGridLayout(glw);
    gl->addWidget(buttonBox);

    full->addWidget(glw);
    gl->setAlignment(Qt::AlignBottom);
    full->setMenuBar(bar);

    pass->setFocus();

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

    keyFilePath = keyEdit->text();
    keyFile = !keyFilePath.isEmpty();

    if (keyFile && !QFile::exists(keyFilePath)) {
        genKey(keyFilePath);
    }

    checksum = checksumBox->currentIndex();
    hash = hashBox->currentIndex();
    hashIters = hashIterBox->value();
    encryption = encryptionBox->currentIndex();
    name = nameEdit->text();
    desc = descEdit->text();

    if (name.isEmpty()) {
        name = "None";
    }

    if (desc.isEmpty()) {
        desc = "None";
    }

    memoryUsage = memBox->value();
    compress = compressBox->isChecked();
    clearSecs = clearBox->value();

    if (create) {
        std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption), Botan::ENCRYPTION);

        Botan::AutoSeeded_RNG rng;
        ivLen = enc->default_nonce_length();
        iv = rng.random_vec(ivLen);

        passw = getPw(pw);
    }
    save();
    if (!create && !pw.isEmpty()) {
        passw = getPw(pw);
    }

    return true;

}

bool Database::open() {
    if (QFile::exists(path)) {
        if (!parse()) {
            return false;
        }

        if (stList.empty()) {
            try {
                QString p = decrypt(" to login");
                if (p.isEmpty()) {
                    return false;
                }
            } catch (std::exception& e) {
                displayErr(e.what());
                return false;
            }
        }

        for (const QString &line : toStr(stList).split('\n')) {
            if (line.isEmpty()) {
                continue;
            }
            QSqlQuery q(db);
            bool ok = q.exec(line);
            if (!ok) {
               displayErr("Warning: Error during database initialization: " + q.lastError().text());
            }
        }
        get();
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

int Database::backup() {
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Backup Location"), "", fileExt);
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
QAction *Database::addButton(const char *text, const char *icon, const char *whatsThis, QKeySequence shortcut, Func func) {
    QAction *action = new QAction(QIcon::fromTheme(tr(icon)), tr(text));
    action->setWhatsThis(tr(whatsThis));
    action->setShortcut(shortcut);
    QObject::connect(action, &QAction::triggered, func);
    return action;
}
