#include <QMessageBox>
#include <QSqlRecord>
#include <QSqlQuery>
#include <QSqlField>
#include <QSqlError>

#include "database.hpp"
#include "entry.hpp"
#include "gui/password_widget.hpp"
#include "gui/password_generator_dialog.hpp"
#include "util/data_stream.hpp"

Entry *Database::entryNamed(const QString &t_name) {
    for (Entry *e : this->m_entries) {
        if (e->name() == t_name) {
            return e;
        }
    }

    return nullptr;
}

Entry *Database::entryWithPassword(const QString &t_pass) {
    for (Entry *e : this->m_entries) {
        if (e->fieldNamed("password")->dataStr() == t_pass) {
            return e;
        }
    }

    return nullptr;
}

// Generates entries from SQLite statements.
void Database::get() {
    setEntries({});

    for (const QString &tbl : db.tables()) {
        QSqlQuery q(db);
        q.exec("SELECT * FROM " + tbl);
        q.next();
        QList<Field *> fields;
        QSqlRecord rec = q.record();
        if (qApp->property("verbose").toBool()) {
            qDebug() << "generating entry from table" << tbl;
            qDebug() << rec;
        }

        for (const int i : range(0, rec.count())) {
            const QString vName = rec.fieldName(i);

            if (i == 3 && vName.toLower() == "password") {
                fields.emplaceBack(new Field("Notes", rec.value(4).toString().replace(" || char(10) || ", "\n"), QMetaType::QByteArray));
                fields.emplaceBack(new Field("Password", rec.value(3), QMetaType::QString));

                Entry *entry = new Entry(fields, this);
                return addEntry(entry);
            } else {
                const QString val = rec.value(i).toString().replace(" || char(10) || ", "\n");
                const QMetaType::Type id = static_cast<QMetaType::Type>(rec.field(i).metaType().id());

                fields.emplaceBack(new Field(vName, val, id));
            }
        }

        Entry *entry = new Entry(fields, this);
        addEntry(entry);
    }
}

// Generates SQLite statements from entries.
bool Database::saveSt() {
    for (const QString &tbl : db.tables()) {
        if (qApp->property("verbose").toBool()) {
            qDebug() << "deleting table" << tbl;
        }
        db.exec("DROP TABLE \"" + tbl + '"');
    }
    stList = "";

    for (Entry *entry : m_entries) {
        if (entry->name().isEmpty()) {
            continue;
        }

        QString createStr = "CREATE TABLE '" + entry->fieldAt(0)->dataStr() + "' (";
        QString insertStr = "INSERT INTO '" + entry->fieldAt(0)->dataStr().replace('"', '\'') + "' (";
        QString valueStr = ") VALUES (";

        for (const int i : range(0, static_cast<int>(entry->fieldLength()))) {
            Field *field = entry->fieldAt(i);
            QString fName = field->name();
            fName.replace('"', '\'');

            const QList<QMetaType::Type> varTypes = {QMetaType::QString, QMetaType::Double, QMetaType::Int, QMetaType::QByteArray};
            const QList<QString> sqlTypes = {"text", "real", "integer", "blob"};

            createStr += fName + ' ' + sqlTypes[varTypes.indexOf(field->type())];
            insertStr += fName;

            QString quote = field->type() == QMetaType::QString || field->isMultiLine() ? "\"" : "";
            valueStr += quote + field->dataStr().replace('"', '\'').replace('\n', " || char(10) || ") + quote;

            createStr += ", ";
            insertStr += ", ";
            valueStr += ", ";
        }

        createStr.chop(2);
        insertStr.chop(2);
        valueStr.chop(2);

        db.exec(createStr + ')');
        db.exec(insertStr + valueStr + ')');
        stList += createStr + ")\n" + insertStr + valueStr + ")\n";
    }
    return true;
}

// Add a new entry.
void Database::add() {
    Entry *entry = new Entry({}, this);
    addEntry(entry);

    entry->edit();
}

// Hashes a provided password.
VectorUnion Database::hashPw(VectorUnion password) {
    const std::string hashChoice = Constants::hashMatch[hash];

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

    secvec ptr(512);
    pHash->derive_key(ptr.data(), ptr.size(), static_cast<const char*>(password), password.size(), iv.data(), ivLen);
    password = ptr;
    if (qApp->property("verbose").toBool()) {
        qDebug() << hashIters << hashChoice.data() << hmacChoice.data() << iv << ivLen << password << Qt::endl;
        qDebug() << "After Hashing:" << Botan::hex_encode(ptr).data() << Qt::endl;
    }

    return password;
}

// Hashes and derives a provided password.
secvec Database::getPw(VectorUnion password) {
    const std::string hmacChoice = getCS();

    if (hash < 3) {
        password = hashPw(password);
    }

    auto enc = makeEncryptor();
    secvec ptr(enc->maximum_keylength());
    auto ph = Botan::PasswordHashFamily::create("PBKDF2(" + hmacChoice + ')')->default_params();

    ph->derive_key(ptr.data(), ptr.size(), static_cast<const char*>(password), password.size(), iv.data(), ivLen);
    if (qApp->property("verbose").toBool()) {
        qDebug() << iv;
        qDebug() << "After Derivation:" << Botan::hex_encode(ptr).data() << Qt::endl;
    }

    return ptr;
}

VectorUnion Database::encryptedData() {
    auto enc = makeEncryptor();
    enc->set_key(passw);
    if (qApp->property("verbose").toBool()) {
        qDebug() << "STList before saveSt:" << stList.asStdStr().data();
    }

    saveSt();

    if (qApp->property("verbose").toBool()) {
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

// Encrypts data and writes it.
void Database::encrypt() {
    DataStream pd(path.asStdStr(), std::fstream::binary | std::fstream::trunc);

    pd << "PD++";

    pd << Constants::maxVersion;
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
    if (qApp->property("verbose").toBool()) {
        qDebug() << "Data (Encryption):" << data.encoded().asQStr();
    }

    pd << data;
    pd.finish();
}

std::pair<VectorUnion, int> Database::decryptData(VectorUnion t_data, const VectorUnion &mpass, const bool convert) {
    typedef std::pair<VectorUnion, int> vPair;
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
                return vPair({}, 3);
            }
        }
    }

    auto decr = makeDecryptor();

    decr->set_key(vPtr);
    decr->start(iv);

    if (qApp->property("verbose").toBool()) {
        qDebug() << "Data (Decryption):" << t_data.encoded().asQStr();
    }

    try {
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

        return vPair(t_data, true);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return vPair({}, false);
    }
}

// Verifies if a password is correct.
int Database::verify(const VectorUnion &mpass, const bool convert) {
    if (convert) {
        QFile f(path.asQStr());
        f.open(QIODevice::ReadOnly);

        const VectorUnion t_iv = f.readLine().trimmed();
        VectorUnion ivd;
        try {
            ivd = t_iv.decoded();
        } catch (...) {
            return false;
        }

        this->iv = ivd;
        this->name = QString(basename(static_cast<const char*>(path))).split('.')[0];;
        this->desc = "Converted from old database format.";

        auto vDataPair = decryptData(f.readAll(), mpass, true);
        bool vDataValid = vDataPair.second;

        if (!vDataValid) {
            return false;
        }

        VectorUnion vData = vDataPair.first;
        this->stList = vData;

        for (const QString &s : vData.asQStr().split('\n')) {
            if (s.isEmpty()) {
                continue;
            }

            QSqlQuery finalQ(db);

            if (!finalQ.exec(s)) {
                displayErr("Warning: SQL execution error during database conversion: " + finalQ.lastError().text() + "\nQuery: " + s);
            }

            finalQ.finish();
        }

        get();
        saveSt();

        encrypt();
        f.close();
        return true;
    }

    return decryptData(data, mpass).second;
}

// Prompts for password and decrypts the data if correct.
bool Database::decrypt(PasswordOptionsFlag options) {
    PasswordWidget *di = new PasswordWidget(this, options);
    if (!di->setup()) {
        return false;
    }

    di->show();
    return true;
}

// Parses parameters from database file.
bool Database::parse() {
    QFile f(path.asQStr());
    f.open(QIODevice::ReadOnly);
    QDataStream q(&f);

    char readData[4];
    q.readRawData(readData, 4);

    if (std::string(readData, 4) != "PD++") {
        PasswordWidget *di = new PasswordWidget(this, PasswordOptions(PasswordOptions::Convert | PasswordOptions::Open));
        if (!di->setup()) {
            return showErr("Invalid magic number. Should be PD++.");
        }

        di->show();

        return true;
    }

    q >> version;
    if (version > Constants::maxVersion) {
        return showErr("Invalid version number.");
    }

    q >> hmac;
    if (hmac >= Constants::hmacMatch.size()){
        return showErr("Invalid HMAC option.");
    }

    if (version < 6) {
        q.skipRawData(1);
    }

    q >> hash;
    if (hash >= Constants::hashMatch.size()){
        return showErr("Invalid hash option.");
    }

    if (hash != 3) {
        q >> hashIters;
    }

    q >> keyFile;

    q >> encryption;
    if (encryption >= Constants::encryptionMatch.size()){
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
    iv = VectorUnion(ivc, static_cast<int>(ivLen));

    name = QString(f.readLine()).trimmed();
    desc = QString(f.readLine()).trimmed();

    const qint64 available = f.bytesAvailable();
    char *datac = new char[available];

    q.readRawData(datac, static_cast<int>(available));
    data = VectorUnion(datac, static_cast<int>(available));

    return true;
}

bool Database::open() {
    window->show();
    if (QFile::exists(path.asQStr())) {
        if (!parse()) {
            return false;
        }

        if (stList.empty()) {
            try {
                if (!decrypt(PasswordOptions::Open)) {
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
    const QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Save Location"), "", Constants::fileExt);
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
