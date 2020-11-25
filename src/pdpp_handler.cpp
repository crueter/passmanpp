#include "pdpp_handler.h"
#include <QDebug>

void showMessage(std::string msg) {
    QMessageBox box;
    box.setText(QString::fromStdString(msg));
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

void encryptData(std::string path, int checksum, int deriv, int hash, int hashIters, bool keyFile, int encryption, Botan::secure_vector<uint8_t> uuid, std::string name, std::string desc, std::string data, std::string password, Botan::secure_vector<uint8_t> iv) {
    std::ofstream pd(path, std::ios_base::binary | std::ios_base::trunc | std::ios_base::out | std::ios_base::in);
    std::string magic, gs, version, writeData;

    magic = atos(0x11U);
    gs = atos(0x1DU);
    version = atos(MAX_SUPPORTED_VERSION_NUMBER);

    writeData = "PD++" + magic + magic + gs + version + atos(checksum) + atos(deriv) + atos(hash) + atos(hashIters) + atos(keyFile + 1) + atos(encryption) + gs;

    std::string sUuid(uuid.begin(), uuid.end());
    std::string uuidLen = atos(sUuid.length());

    std::unique_ptr<Botan::Compression_Algorithm> nameComp = Botan::Compression_Algorithm::create("gzip");
    Botan::secure_vector<uint8_t> uName(name.begin(), name.end());
    nameComp->start();
    nameComp->finish(uName);

    name = std::string(uName.begin(), uName.end());
    std::string nameLen = atos(name.length());

    std::unique_ptr<Botan::Compression_Algorithm> descComp = Botan::Compression_Algorithm::create("gzip");
    Botan::secure_vector<uint8_t> uDesc(desc.begin(), desc.end());
    descComp->start();
    descComp->finish(uDesc);

    desc = std::string(uDesc.begin(), uDesc.end());
    std::string descLen = atos(desc.length());

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption - 1), Botan::ENCRYPTION);

    Botan::secure_vector<uint8_t> rIv;
    if (iv == Botan::secure_vector<uint8_t>{}) {
        Botan::AutoSeeded_RNG rng;
        rIv = rng.random_vec(enc->default_nonce_length());
    } else {
        rIv = iv;
    }
    std::string sIv(rIv.begin(), rIv.end());
    std::string ivLen = atos(sIv.length());

    std::string checksumChoice = checksumMatch[checksum - 1];
    if (checksumChoice == "Skein-512")
        checksumChoice = "Skein-512(256, " + std::string(uuid.begin(), uuid.end()) + ")";

    std::unique_ptr<Botan::HashFunction> hash3(Botan::HashFunction::create(checksumChoice));

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
        pHash->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), uuid.data(), int(uuidLen[0]));
        password = std::string(ptr.begin(), ptr.end());
    }

    Botan::secure_vector<uint8_t> ptr(32);
    std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(" + checksumChoice + ")")->default_params();

    ph->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), uuid.data(), uuid.size());

    enc->set_key(ptr);

    Botan::secure_vector<uint8_t> pt(data.data(), data.data() + data.length());
    std::unique_ptr<Botan::Compression_Algorithm> ptComp = Botan::Compression_Algorithm::create("gzip");

    ptComp->start();
    ptComp->finish(pt);

    std::cout << Botan::hex_encode(rIv) << std::endl;

    enc->start(rIv);
    enc->finish(pt);

    std::string pts(pt.begin(), pt.end());

    writeData += ivLen + std::string(iv.begin(), iv.end()) + uuidLen + sUuid + nameLen + name + descLen + desc + pts;

    pd.seekp(0);
    pd << writeData;
    pd.flush();
    pd.close();
}

bool saveFile(std::string path, std::string txt, std::string password) {
    std::vector<std::string> parsed = decryptData(path, txt, password);
    int checksum = std::stoi(parsed[3]);
    int deriv = std::stoi(parsed[4]);
    int hash = std::stoi(parsed[5]);
    int hashIters = std::stoi(parsed[6]);
    bool keyFile = bool(std::stoi(parsed[7]));
    int encryption = std::stoi(parsed[8]);
    std::string iv = parsed[9];
    std::string uuid = parsed[10];
    std::string name = parsed[11];
    std::string desc = parsed[12];
    std::string data = parsed[0];

    std::string mpass;
    if (password != "") {
        mpass = parsed[1];
    } else {
        mpass = password;
    }
    encryptData(path, checksum, deriv, hash, hashIters, keyFile, encryption, Botan::secure_vector<uint8_t>(uuid.begin(), uuid.end()), name, desc, data, mpass, Botan::secure_vector<uint8_t>(iv.begin(), iv.end()));
    return true;
}

std::vector<std::string> convert(std::string path) {
    std::ifstream pd(path, std::ios_base::binary | std::ios_base::out);
    std::string iv;
    std::getline(pd, iv);
    std::vector<uint8_t> ivd;
    try {
        ivd = Botan::hex_decode(iv);
    } catch (...) {
        return {};
    }
    std::string r(std::istreambuf_iterator<char>{pd}, {});

    if (ivd.size() != 12) return {};

    showMessage("This database may be for an older version of passman++. I will try to convert it now.");

    std::string password;
    Botan::secure_vector<uint8_t> data;

    while (true) {
        data = Botan::secure_vector<uint8_t>(r.begin(), r.end());
        QString pass = QInputDialog::getText(nullptr, QWidget::tr("Enter your password"), QWidget::tr(std::string("Please enter your master password to convert the database.").c_str()), QLineEdit::Password);
        password = pass.toStdString();

        Botan::secure_vector<uint8_t> mptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();

        ph->derive_key(mptr.data(), mptr.size(), password.c_str(), password.size(), ivd.data(), ivd.size());

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create("AES-256/GCM", Botan::DECRYPTION);

        decr->set_key(mptr);
        decr->start(ivd);
        try {
            decr->finish(data);
            break;
        }  catch (...) {
            displayErr("Wrong password, try again.");
        }
    }

    int uuidLen = randombytes_uniform(80);
    Botan::AutoSeeded_RNG rng;
    Botan::secure_vector<uint8_t> uuid = rng.random_vec(uuidLen);

    std::string name = split(std::string(basename(path.c_str())), '.')[0];
    std::string rdata(data.begin(), data.end());

    std::cout << iv << std::endl;

    encryptData(path, 1, 1, 1, 8, false, 1, uuid, name, "Converted from old database format.", rdata, password, Botan::secure_vector<uint8_t>(ivd.begin(), ivd.end()));
    pd.close();
    return {rdata, password};
}

std::vector<std::string> decryptData(std::string path, std::string txt, std::string password) {
    std::string mpass;
    //    return {dbVer, checksum, deriv, hash, hashIters, keyFile, encryption, iv, uuid, name, desc, data};

    std::vector<std::string> parsed = parseFile(path);

    if (parsed == std::vector<std::string>{}) {
        throw std::ios_base::failure("This is not a valid database file.");
    }

    int checksum = std::stoi(parsed[1]);
    int deriv = std::stoi(parsed[2]);
    int hash = std::stoi(parsed[3]);
    int hashIters = std::stoi(parsed[4]);
    bool keyFile = bool(std::stoi(parsed[5]));
    int encryption = std::stoi(parsed[6]);
    std::string iv = parsed[7];
    std::string uuid = parsed[8];
    std::string name = parsed[9];
    std::string desc = parsed[10];
    std::string data = parsed[11];

    int uuidLen = uuid.length();
    Botan::secure_vector<uint8_t> uuidc(uuid.begin(), uuid.end());

    Botan::secure_vector<uint8_t> ivc(iv.begin(), iv.end());

    qDebug() << QString::fromStdString(iv);

    Botan::secure_vector<uint8_t> rp;
    std::string mpassword, origPass;

    while(1) {
        rp = Botan::secure_vector<uint8_t>(data.begin(), data.end());

        if (password == "") {
            QString pass = QInputDialog::getText(nullptr, QWidget::tr("Enter your password"), QWidget::tr(std::string("Please enter your master password" +  txt + ".").c_str()), QLineEdit::Password);
            mpass = pass.toStdString();
        } else {
            mpass = password;
        }
        mpassword = mpass;

        std::string checksumChoice = checksumMatch[checksum - 1];
        if (checksumChoice == "Skein-512")
            checksumChoice = "Skein-512(256, " + uuid + ")";

        std::unique_ptr<Botan::HashFunction> hash3(Botan::HashFunction::create(checksumChoice));

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
            pHash->derive_key(ptr.data(), ptr.size(), mpass.c_str(), mpass.size(), uuidc.data(), int(uuidLen));
            mpass = std::string(ptr.begin(), ptr.end());
        }

        Botan::secure_vector<uint8_t> ptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(" + checksumChoice + ")")->default_params();

        Botan::secure_vector<uint8_t> vUuid(uuid.begin(), uuid.end());
        ph->derive_key(ptr.data(), ptr.size(), mpass.c_str(), mpass.size(), vUuid.data(), vUuid.size());

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create(encryptionMatch.at(encryption - 1), Botan::DECRYPTION);

        decr->set_key(ptr);
        decr->start(ivc);
        std::cout << Botan::hex_encode(ivc) << std::endl;
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
    std::vector<std::string> ret = {rpr, mpassword};
    ret.insert(ret.end(), parsed.begin(), parsed.end());
    return ret;
}


std::vector<std::string> showErr(std::string msg) {
    displayErr("Error: database file is corrupt or invalid.\nDetails: " + msg);
    return {};
}

std::vector<std::string> parseFile(std::string path) {
    std::ifstream pd(path, std::ios_base::binary | std::ios_base::out);
    char readData[65536], len[2];
    pd.read(readData, 6);
    if (readData != "PD++" + atos(0x11U) + atos(0x11U)) {
        std::vector<std::string> yes = convert(path);
        if (yes == std::vector<std::string>{}) {
            return showErr("Invalid magic number \"" + std::string(readData) + "\".");
        }
        return parseFile(path);
    }

    pd.read(readData, 1);
    if (int(readData[0]) != 0x1DU) {
        return showErr("No group separator byte after valid magic number.");
    }

    pd.read(readData, 1);
    if (int(readData[0]) > MAX_SUPPORTED_VERSION_NUMBER) {
        return showErr("Invalid version number.");
    }
    int dbVer = readData[0];

    pd.read(readData, 1);
    try {
        checksumMatch.at(int(readData[0]) - 1);
    }  catch (...) {
        return showErr("Invalid checksum option.");
    }
    int checksum = readData[0];

    pd.read(readData, 1);
    try {
        derivMatch.at(int(readData[0]) - 1);
    }  catch (...) {
        return showErr("Invalid derivation option.");
    }
    int deriv = readData[0];

    pd.read(readData, 1);
    try {
        hashMatch.at(int(readData[0]) - 1);
    }  catch (...) {
        return showErr("Invalid hash option.");
    }
    int hash = readData[0];

    pd.read(readData, 1);
    int hashIters = int(readData[0]);

    pd.read(readData, 1);
    bool keyFile = readData[0] - 1;

    pd.read(readData, 1);
    try {
        encryptionMatch.at(int(readData[0]));
    }  catch (...) {
        return showErr("Invalid encryption option.");
    }
    int encryption = readData[0];

    pd.read(readData, 1);
    if (int(readData[0]) != 0x1DU) {
        return showErr("No group separator byte after database configuration.");
    }

    pd.read(len, 1);

    pd.read(readData, int(len[0]));
    std::string iv(readData);

    pd.read(len, 1);

    pd.read(readData, int(len[0]));
    std::string uuid(readData);

    pd.read(len, 1);

    pd.read(readData, int(len[0]));

    std::unique_ptr<Botan::Decompression_Algorithm> decomp = Botan::Decompression_Algorithm::create("gzip");
    std::string sData(readData, int(len[0]));
    Botan::secure_vector<uint8_t> uData(sData.begin(), sData.end());
    decomp->start();
    decomp->finish(uData);

    std::string name(uData.begin(), uData.end());

    pd.read(len, 1);

    pd.read(readData, int(len[0]));

    std::unique_ptr<Botan::Decompression_Algorithm> decompD = Botan::Decompression_Algorithm::create("gzip");
    std::string sDataD(readData, int(len[0]));
    Botan::secure_vector<uint8_t> uDataD(sDataD.begin(), sDataD.end());
    decompD->start();
    decompD->finish(uDataD);

    std::string desc(uDataD.begin(), uDataD.end());

    std::string data(std::istreambuf_iterator<char>{pd}, {});

    //std::vector<std::string> retVec;
    std::vector<std::string> retVec = {std::to_string(dbVer), std::to_string(checksum), std::to_string(deriv), std::to_string(hash), std::to_string(hashIters), std::to_string(keyFile), std::to_string(encryption), iv, uuid, name, desc, data};

    ;
    pd.close();
    return retVec;
}
