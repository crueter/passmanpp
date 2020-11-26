#include "database.h"
#include "constants.h"
#include "entry_handler.h"

#include <QInputDialog>

Database::Database() {}

void showMessage(std::string msg) {
    QMessageBox box;
    box.setText(QString::fromStdString(msg));
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}


std::string Database::getPw(std::string password) {
    std::string uuidLen = atos(uuid.length());

    Botan::secure_vector<uint8_t> vUuid = toVec(uuid);

    std::string checksumChoice = checksumMatch[checksum - 1];
    if (checksumChoice == "Skein-512")
        checksumChoice = "Skein-512(256, " + iv + ")";

    std::string hashChoice = hashMatch[hash - 1];

    if (hashChoice != "No hashing, only derivation") {
        std::unique_ptr<Botan::PasswordHashFamily> pfHash = Botan::PasswordHashFamily::create(hashChoice);
        std::unique_ptr<Botan::PasswordHash> pHash;
        if (hashChoice == "Argon2id") {
            pHash = pfHash->from_params(hashIters, 80000, 1);
        } else {
            pHash = pfHash->from_params(hashIters);
        }

        Botan::secure_vector<uint8_t> ptr(1024);
        pHash->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), vUuid.data(), int(uuidLen[0]));
        password = toStr(ptr);
    }

    Botan::secure_vector<uint8_t> ptr(32);
    std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(" + checksumChoice + ")")->default_params();

    ph->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), vUuid.data(), uuid.size());
    return toStr(ptr);
}

void Database::encrypt(std::string password) {
    std::ofstream pd(path, std::ios_base::binary | std::ios_base::trunc | std::ios_base::out | std::ios_base::in);
    pd.seekp(0);
    pd << "PD++";
    std::string magic, gs, version;

    magic = atos(0x11U);
    pd << magic << magic;

    gs = atos(0x1DU);
    pd << gs;

    version = atos(MAX_SUPPORTED_VERSION_NUMBER);
    pd << version << atos(checksum) << atos(deriv) << atos(hash) << atos(hashIters) << atos(keyFile + 1) << atos(encryption) << gs;

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption - 1), Botan::ENCRYPTION);

    if (iv == "") {
        Botan::AutoSeeded_RNG rng;
        iv = toStr(rng.random_vec(enc->default_nonce_length()));
    }

    std::string ivLen = atos(iv.length());
    pd << ivLen << iv;

    std::string uuidLen = atos(uuid.length());
    pd << uuidLen << uuid;

    std::unique_ptr<Botan::Compression_Algorithm> nameComp = Botan::Compression_Algorithm::create("gzip");
    Botan::secure_vector<uint8_t> uName = toVec(name);
    nameComp->start();
    nameComp->finish(uName);

    name = toStr(uName);
    std::string nameLen = atos(name.length());
    pd << nameLen << name;

    std::unique_ptr<Botan::Compression_Algorithm> descComp = Botan::Compression_Algorithm::create("gzip");
    Botan::secure_vector<uint8_t> uDesc = toVec(desc);
    descComp->start();
    descComp->finish(uDesc);

    desc = toStr(uDesc);
    std::string descLen = atos(desc.length());
    pd << descLen << desc;

    std::string ptr = getPw(password);
    Botan::secure_vector<uint8_t> vPassword = toVec(ptr);

    enc->set_key(vPassword);

    Botan::secure_vector<uint8_t> pt(stList.data(), stList.data() + stList.length());
    std::unique_ptr<Botan::Compression_Algorithm> ptComp = Botan::Compression_Algorithm::create("gzip");

    ptComp->start();
    ptComp->finish(pt);

    enc->start(toVec(iv));
    enc->finish(pt);

    std::string pts = toStr(pt);
    data = pts;
    pd << pts;

    pd.flush();
    pd.close();
}

std::string Database::decrypt(std::string txt, std::string password) {
    Botan::secure_vector<uint8_t> uuidc = toVec(uuid), ivc = toVec(iv), rp;
    std::string mpass;

    while(1) {
        rp = toVec(data);

        if (password == "") {
            QString pass = QInputDialog::getText(nullptr, QWidget::tr("Enter your password"), QWidget::tr(std::string("Please enter your master password" +  txt + ".").c_str()), QLineEdit::Password);
            mpass = pass.toStdString();
            if (pass == "") {
                std::cout << "Cancelled." << std::endl;
                return mpass;
            }
        } else {
            mpass = password;
        }

        std::string ptr = getPw(mpass);

        Botan::secure_vector<uint8_t> vPtr = toVec(ptr);

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create(encryptionMatch.at(encryption - 1), Botan::DECRYPTION);

        decr->set_key(vPtr);
        decr->start(ivc);

        try {
            decr->finish(rp);

            std::unique_ptr<Botan::Decompression_Algorithm> dataDe = Botan::Decompression_Algorithm::create("gzip");
            dataDe->start();
            dataDe->finish(rp);
        } catch (std::exception& e) {
            displayErr("Wrong password, please try again.\n" + std::string(e.what()));
            continue;
        }
        break;
    }
    std::string rpr(rp.begin(), rp.end());
    stList = rpr;
    return mpass;
}

bool Database::save(std::string password) {
    std::string pw = decrypt(" to save");
    if (pw == "") return false;

    std::string mpass;
    if (pw != "") {
        mpass = pw;
    } else {
        mpass = password;
    }
    stList = glob_stList;
    encrypt(mpass);
    modified = false;
    return true;
}

bool Database::saveAs(std::string savePath) {
    path = savePath;
    return save();
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

    if (ivd.size() != 12) return false;

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

    int uuidLen = randombytes_uniform(80);
    Botan::AutoSeeded_RNG rng;
    Botan::secure_vector<uint8_t> uuid = rng.random_vec(uuidLen);

    std::string name = split(std::string(basename(path.c_str())), '.')[0];
    std::string rdata = toStr(vData);

    this->checksum = 1;
    this->deriv = 1;
    this->hash = 1;
    this->hashIters = 8;
    this->keyFile = false;
    this->encryption = 1;
    this->uuid = std::string(uuid.begin(), uuid.end());
    this->name = name;
    desc = "Converted from old database format.";
    this->stList = rdata;
    this->iv = std::string(ivd.begin(), ivd.end());

    encrypt(password);

    pd.close();
    return true;
}

bool Database::showErr(std::string msg) {
    displayErr("Error: database file is corrupt or invalid.\nDetails: " + msg);
    return {};
}

bool Database::parse() {
    std::ifstream pd(path, std::ios_base::binary | std::ios_base::out);
    char readData[65536], len[2];
    pd.read(readData, 6);
    if (readData != "PD++" + atos(0x11U) + atos(0x11U)) {
        bool conv = convert();
        if (!conv) {
            return showErr("Invalid magic number \"" + std::string(readData) + "\".");
        }
        return true;
    }

    pd.read(readData, 1);
    if (int(readData[0]) != 0x1DU) {
        return showErr("No group separator byte after valid magic number.");
    }

    pd.read(readData, 1);
    if (int(readData[0]) > MAX_SUPPORTED_VERSION_NUMBER) {
        return showErr("Invalid version number.");
    }
    version = readData[0];

    pd.read(readData, 1);
    try {
        checksumMatch.at(int(readData[0]) - 1);
    }  catch (...) {
        return showErr("Invalid checksum option.");
    }
    checksum = readData[0];

    pd.read(readData, 1);
    try {
        derivMatch.at(int(readData[0]) - 1);
    }  catch (...) {
        return showErr("Invalid derivation option.");
    }
    deriv = readData[0];

    pd.read(readData, 1);
    try {
        hashMatch.at(int(readData[0]) - 1);
    }  catch (...) {
        return showErr("Invalid hash option.");
    }
    hash = readData[0];

    pd.read(readData, 1);
    hashIters = int(readData[0]);

    pd.read(readData, 1);
    keyFile = readData[0] - 1;

    pd.read(readData, 1);
    try {
        encryptionMatch.at(int(readData[0]));
    }  catch (...) {
        return showErr("Invalid encryption option.");
    }
    encryption = readData[0];

    pd.read(readData, 1);
    if (int(readData[0]) != 0x1DU) {
        return showErr("No group separator byte after database configuration.");
    }

    pd.read(len, 1);

    pd.read(readData, int(len[0]));
    iv = readData;

    pd.read(len, 1);

    pd.read(readData, int(len[0]));
    uuid = readData;

    pd.read(len, 1);

    pd.read(readData, int(len[0]));

    std::unique_ptr<Botan::Decompression_Algorithm> decomp = Botan::Decompression_Algorithm::create("gzip");
    std::string sData(readData, int(len[0]));
    Botan::secure_vector<uint8_t> uData = toVec(sData);

    decomp->start();
    decomp->finish(uData);

    name = toStr(uData);

    pd.read(len, 1);

    pd.read(readData, int(len[0]));

    std::unique_ptr<Botan::Decompression_Algorithm> decompD = Botan::Decompression_Algorithm::create("gzip");
    std::string sDataD(readData, int(len[0]));
    Botan::secure_vector<uint8_t> uDataD = toVec(sDataD);
    decompD->start();
    decompD->finish(uDataD);

    desc = toStr(uDataD);
    data = std::string(std::istreambuf_iterator<char>{pd}, {});

    pd.close();
    return true;
}
