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

#include "entry.hpp"
#include "gui/database_window.hpp"
#include "gui/config_dialog.hpp"
#include "gui/password_dialog.hpp"
#include "util/data_stream.hpp"

void showMessage(const QString &msg) {
    QMessageBox box;

    box.setText(msg);
    box.setStandardButtons(QMessageBox::Ok);
    box.setTextInteractionFlags(Qt::LinksAccessibleByMouse | Qt::TextSelectableByMouse);

    box.exec();
}

const std::string Database::getCS() {
    auto enc = makeEncryptor();
    std::string hmacChoice(hmacMatch[hmac]);

    if (hmacChoice != "SHA-512") {
        hmacChoice += '(' + std::to_string(enc->maximum_keylength() * 8) + ')';
    }
    return hmacChoice;
}

void Database::get() {
    setEntries({});
    for (QSqlQuery q : selectAll()) {
        while (q.next()) {
            QList<Field *> fields;

            for (const int i : range(0, q.record().count())) {
                const QString val = q.record().value(i).toString().replace(" || char(10) || ", "\n");
                const QString vName = q.record().fieldName(i);
                const QMetaType::Type id = static_cast<QMetaType::Type>(q.record().field(i).metaType().id());

                Field *f = new Field(vName, val, id);
                fields.push_back(f);
            }

            Entry *entry = new Entry(fields, this);
            addEntry(entry);
        }
    }
}

int Database::add(QTableWidget *table) {
    Entry *entry = new Entry({}, this);
    addEntry(entry);

    return entry->edit(nullptr, table);
}

int Database::edit() {
    DatabaseWindow *di = new DatabaseWindow(this);
    di->setup();
    return di->exec();
}

bool Database::saveSt(const bool exec) {
    const VectorUnion execSt = [&] {
        VectorUnion t_execSt{};
        for (Entry *entry : m_entries) {
            t_execSt += entry->getCreate();
        }

        return t_execSt;
    }();

    if (exec) {
        QSqlQuery delQuery(db);
        delQuery.exec("SELECT 'DROP TABLE \"' || name || '\"' FROM sqlite_master WHERE type = 'table'");

        const QString delSt = [&] {
            QString t_delSt;
            while (delQuery.next()) {
                t_delSt += delQuery.value(0).toString() + '\n';
            }

            return t_delSt;
        }();

        delQuery.finish();
        execAll(delSt);

        execAll(execSt.asQStr());
    }
    this->stList = execSt;
    return true;
}

VectorUnion Database::hashPw(VectorUnion password) {
    const std::string hashChoice = hashMatch[hash];

    auto pfHash = Botan::PasswordHashFamily::create(hashChoice);
    auto pHash = [&] {
        std::unique_ptr<Botan::PasswordHash> t_pHash;
        switch (hash) {
            case 0: {
                t_pHash = pfHash->from_params(memoryUsage * 1000, hashIters, 1);
                break;
            } case 2: {
                t_pHash = pfHash->from_params(32768, hashIters, 1);
                break;
            } default: {
                t_pHash = pfHash->from_params(hashIters);
                break;
            }
        }

        return t_pHash;
    }();

    const std::string hmacChoice = getCS();

    if (verbose) {
        qDebug() << hashIters << hashChoice.data() << hmacChoice.data() << iv << ivLen << password << Qt::endl;
    }

    secvec ptr(512);
    pHash->derive_key(ptr.data(), ptr.size(), static_cast<const char*>(password), password.size(), iv.data(), ivLen);
    password = ptr;
    if(verbose) {
        qDebug() << "After Hashing:" << Botan::hex_encode(ptr).data() << Qt::endl;
    }

    return password;
}

secvec Database::getPw(VectorUnion password) {
    const std::string hmacChoice = getCS();

    if (hash < 3) {
        password = hashPw(password);
    }

    auto enc = makeEncryptor();
    secvec ptr(enc->maximum_keylength());
    auto ph = Botan::PasswordHashFamily::create("PBKDF2(" + hmacChoice + ')')->default_params();

    ph->derive_key(ptr.data(), ptr.size(), static_cast<const char*>(password), password.size(), iv.data(), ivLen);
    if (verbose) {
        qDebug() << iv;
        qDebug() << "After Derivation:" << Botan::hex_encode(ptr).data() << Qt::endl;
    }

    return ptr;
}

VectorUnion Database::getKey() {
    QFile kf(keyFilePath.asQStr());
    kf.open(QIODevice::ReadOnly);
    QTextStream key(&kf);

    return key.readAll();
}

VectorUnion Database::encryptedData() {
    auto enc = makeEncryptor();
    enc->set_key(passw);
    if (verbose) {
        qDebug() << "STList before saveSt:" << stList.asStdStr().data();
    }

    saveSt();

    if (verbose) {
        qDebug() << "STList after saveSt:" << stList.asStdStr().data();
    }

    VectorUnion pt = stList;

    if (compress) {
        auto ptComp = Botan::Compression_Algorithm::create("gzip");

        ptComp->start();
        ptComp->finish(pt);
    }

    enc->start(iv);
    enc->finish(pt);

    if (keyFile) {
        const VectorUnion keyPw = this->getKey();

        auto keyEnc = makeEncryptor();

        keyEnc->set_key(getPw(keyPw));
        keyEnc->start(iv);
        keyEnc->finish(pt);
    }

    return pt;
}

void Database::encrypt() {
    DataStream pd(path.asStdStr(), std::fstream::binary | std::fstream::trunc);

    pd << "PD++";

    pd << maxVersion;
    pd << hmac;
    pd << hash;

    if (hash != 3) {
        pd << hashIters;
    }

    pd << keyFile;
    pd << encryption;

    if (hash == 0) {
        pd << memoryUsage;
    }

    pd << clearSecs;
    pd << compress;

    pd << iv;

    pd << name << '\n';
    pd << desc << '\n';

    data = this->encryptedData();
    if (verbose) {
        qDebug() << "Data (Encryption):" << data.encoded().data();
    }

    pd << data;
    pd.finish();
}

VectorUnion Database::decryptData(VectorUnion t_data, const VectorUnion &mpass, const bool convert) {
    VectorUnion vPtr;
    if (convert) {
        secvec mptr(32);
        auto ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();

        ph->derive_key(mptr.data(), mptr.size(), static_cast<const char*>(mpass), mpass.size(), iv.data(), iv.size());
        vPtr = mptr;
    } else {
        vPtr = getPw(mpass);

        if (keyFile) {
            VectorUnion keyPw = this->getKey();
            auto keyDec = makeDecryptor();

            keyDec->set_key(getPw(keyPw));
            keyDec->start(iv);

            try {
                keyDec->finish(t_data);
            } catch (std::exception& e) {
                std::cerr << e.what() << std::endl;
                return {};
            }
        }
    }

    auto decr = makeDecryptor();

    decr->set_key(vPtr);
    decr->start(iv);

    try {
        if (verbose) {
            qDebug() << "Data (Decryption):" << t_data.encoded().data();
        }
        decr->finish(t_data);

        if (convert) {
            this->passw = getPw(mpass);
        } else {
            if (compress) {
                auto dataDe = Botan::Decompression_Algorithm::create("gzip");
                dataDe->start();
                dataDe->finish(t_data);
            }

            this->passw = vPtr;
            this->stList = t_data;
        }
        return t_data;
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return {};
    }
}

int Database::verify(const VectorUnion &mpass, const bool convert) {
    if (convert) {
        QFile f(path.asQStr());
        f.open(QIODevice::ReadOnly);
        QTextStream pd(&f);

        const VectorUnion t_iv = pd.readLine();
        VectorUnion ivd;
        try {
            ivd = t_iv.decoded();
        } catch (...) {
            return false;
        }

        this->iv = ivd;
        this->name = QString(basename(static_cast<const char*>(path))).split('.')[0];;
        this->desc = "Converted from old database format.";

        const VectorUnion vData = decryptData(pd.readAll(), mpass, true);
        this->stList = vData;
        execAll(vData.asQStr());

        QSqlQuery q(db);
        q.exec("SELECT tbl_name FROM sqlite_master WHERE type='table'");

        while (q.next()) {
            const QString tblName = q.value(0).toString();
            execAll("CREATE TABLE '" + tblName + "_COPY' (Name text, Email text, URL text, Notes blob, Password text)\n"
                    "INSERT INTO '" + tblName + "_COPY' SELECT name, email, url, notes, password FROM '" + tblName + "'\n"
                    "DROP TABLE '" + tblName + "'\n"
                    "ALTER TABLE '" + tblName + "_COPY' RENAME TO '" + tblName + '\'');
        }
        get();
        saveSt();
        db.exec("DROP TABLE data");

        encrypt();
        f.close();
        return true;
    }

    decryptData(data, mpass);
    return true;
}

const QString Database::decrypt(const QString &txt, const bool convert) {
    PasswordDialog *di = new PasswordDialog(this, convert, txt);
    if (!di->setup()) {
        return "";
    }

    return di->show();
}

bool Database::save() {
    encrypt();

    modified = false;
    return true;
}

bool Database::showErr(const QString &msg) {
    displayErr("Error: database file is corrupt or invalid.\nDetails: " + msg);
    return false;
}

bool Database::parse() {
    QFile f(path.asQStr());
    f.open(QIODevice::ReadOnly);
    QDataStream q(&f);

    char readData[4];
    q.readRawData(readData, 4);

    if (std::string(readData, 4) != "PD++") {
        PasswordDialog *di = new PasswordDialog(this, true, " to convert your database to the new format");
        if (!di->setup()) {
            return showErr("Invalid magic number.");
        }

        di->show();

        return true;
    }

    q >> version;
    if (version > maxVersion) {
        return showErr("Invalid version number.");
    }

    q >> hmac;
    if (hmac >= hmacMatch.size()){
        return showErr("Invalid HMAC option.");
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

    ivLen = makeEncryptor()->default_nonce_length();

    char *ivc = new char[ivLen];
    q.readRawData(ivc, static_cast<int>(ivLen));
    iv = toVec(ivc, static_cast<int>(ivLen));

    name = QString(f.readLine()).trimmed();
    desc = QString(f.readLine()).trimmed();

    const qint64 available = f.bytesAvailable();
    char *datac = new char[available];

    q.readRawData(datac, static_cast<int>(available));
    data = toVec(datac, static_cast<int>(available));

    return true;
}

bool Database::config(const bool create) {
    ConfigDialog *di = new ConfigDialog(this, create);
    di->setup();
    return di->show();
}

bool Database::open() {
    if (QFile::exists(path.asQStr())) {
        if (!parse()) {
            return false;
        }

        if (stList.empty()) {
            try {
                const QString p = decrypt(" to login");
                if (p.isEmpty()) {
                    return false;
                }
            } catch (std::exception& e) {
                displayErr(e.what());
                return false;
            }
        }

        for (const QString &line : stList.asQStr().split('\n')) {
            if (line.isEmpty()) {
                continue;
            }

            QSqlQuery q(db);
            if (!q.exec(line)) {
               displayErr("Warning: Error during database initialization: " + q.lastError().text());
            }
        }
        get();
        return true;
    }
    displayErr("Please enter a valid path!");
    return false;
}

int Database::saveAs() {
    const QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save Location"), "", fileExt);
    if (fileName.isEmpty()) {
        return 3;
    }

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        return 17;
    }

    try {
        path = fileName;
        if (!save()) {
            return false;
        }
    } catch (std::exception& e) {
        displayErr(e.what());
    }
    return true;
}

Entry *Database::entryNamed(const QString &ename) {
    for (Entry *e : m_entries) {
        if (e->name() == ename) {
            return e;
        }
    }

    return new Entry();
}

void Database::addEntry(Entry *entry) {
    m_entries.push_back(entry);
}

bool Database::removeEntry(Entry *entry) {
    return m_entries.removeOne(entry);
}

qsizetype Database::entryLength() {
    return m_entries.length();
}

QList<Entry *> &Database::entries() {
    return m_entries;
}

void Database::setEntries(QList<Entry *> entries) {
    this->m_entries = entries;
}

void Database::redrawTable(QTableWidget *table) {
    int j = 0;
    table->setRowCount(static_cast<int>(this->entryLength()));

    for (Entry *e : this->m_entries) {
        for (const int i : range(0, static_cast<int>(e->fieldLength()))) {
            table->setItem(j, i, new QTableWidgetItem(e->fieldAt(i)->dataStr()));
        }
        ++j;
    }
}

