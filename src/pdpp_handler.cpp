#include "pdpp_handler.h"

#include <fstream>
#include <variant>
#include <sodium.h>
#include <QTranslator>
#include <QInputDialog>

void showMessage(std::string msg) {
    QMessageBox box;
    box.setText(QString::fromStdString(msg));
    box.setStandardButtons(QMessageBox::Ok);
    box.exec();
}

std::vector<std::string> decryptData(std::string path, std::string txt) {
    std::string mpass;
    //    return {dbVer, checksum, deriv, hash, hashIters, keyFile, encryption, iv, uuid, name, desc, data};

    std::vector<std::string> parsed = parseFile(path);

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

    bool convert = false;
    std::vector<std::string> conv{"CONVERT"};

    if (parsed == conv) {
        convert = true;
    } else if (parsed == std::vector<std::string>{}) {
        throw std::ios_base::failure("This is not a valid database file.");
    }

    //std::string r(std::istreambuf_iterator<char>{pdpp}, {});
    Botan::secure_vector<uint8_t> rp;
    std::string password;

    while(1) {
        rp = Botan::secure_vector<uint8_t>(data.begin(), data.end());
        QString pass = QInputDialog::getText(nullptr, QTranslator::tr("Enter your password"), QTranslator::tr(std::string("Please enter your master password" +  txt + ".").c_str()), QLineEdit::Password);
        mpass = pass.toStdString();
        password = mpass;

        std::unique_ptr<Botan::Decompression_Algorithm> nameDe = Botan::Decompression_Algorithm::create("gzip");
        Botan::secure_vector<uint8_t> uName(name.begin(), name.end());
        nameDe->start();
        nameDe->finish(uName);

        name = std::string(uName.begin(), uName.end());

        std::unique_ptr<Botan::Decompression_Algorithm> descDe = Botan::Decompression_Algorithm::create("gzip");
        Botan::secure_vector<uint8_t> uDesc(desc.begin(), desc.end());
        descDe->start();
        descDe->finish(uDesc);

        desc = std::string(uDesc.begin(), uDesc.end());

        std::string checksumChoice = checksumMatch[checksum - 1];
        if (checksumChoice == "Skein-512")
            checksumChoice = "Skein-512(256, " + std::string(uuid.begin(), uuid.end()) + ")";

        std::unique_ptr<Botan::HashFunction> hash3(Botan::HashFunction::create(checksumChoice));

        std::string hashChoice = hashMatch[hash - 1];
        if (hashChoice != "No hashing, only derivation") {
            std::unique_ptr<Botan::PasswordHashFamily> pfHash = Botan::PasswordHashFamily::create(hashChoice);
            std::unique_ptr<Botan::PasswordHash> pHash;
            if (hashChoice == "Argon2id") {
                pHash = pfHash->from_params(1000, 640000);
            } else {
                pHash = pfHash->default_params();
            }
            for (int i = 0; i < hashIters; ++i) {
                Botan::secure_vector<uint8_t> ptr(1024);
                pHash->derive_key(ptr.data(), ptr.size(), mpass.c_str(), mpass.size(), uuidc.data(), int(uuidLen));
                mpass = std::string(ptr.begin(), ptr.end());
            }
        }

        Botan::secure_vector<uint8_t> ptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(" + checksumChoice + ")")->default_params();

        ph->derive_key(ptr.data(), ptr.size(), mpass.c_str(), mpass.size(), ivc.data(), ivc.size());

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create(encryptionMatch.at(encryption - 1), Botan::DECRYPTION);

        decr->set_key(ptr);
        decr->start(ivc);
        try {
            decr->finish(rp);
        } catch (std::exception& e) {
            displayErr("Wrong password, please try again.\n" + std::string(e.what()));
        }
        break;
    }
    std::string rpr(rp.begin(), rp.end());
    return {rpr, password};
}

void encryptData(std::string path, int checksum, int deriv, int hash, int hashIters, bool keyFile, int encryption, Botan::secure_vector<uint8_t> uuid, std::string name, std::string desc, std::string data, std::string password) {
    std::ifstream pd(path, std::ios_base::binary | std::ios_base::in);
    std::string magic, gs, version, writeData;

    magic = atos(0x11U);
    gs = atos(0x1DU);
    version = atos(MAX_SUPPORTED_VERSION_NUMBER);

    std::cout << "u" << std::endl;
    writeData = "PD++" + magic + magic + gs + version + atos(checksum) + atos(deriv) + atos(hash) + atos(hashIters) + atos(keyFile + 1) + atos(encryption) + gs;

    std::string uuidLen = atos(randombytes_uniform(80));

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
    std::string descLen = shortToStr(desc.length());

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create(encryptionMatch.at(encryption - 1), Botan::ENCRYPTION);

    Botan::AutoSeeded_RNG rng;
    Botan::secure_vector<uint8_t> iv = rng.random_vec(enc->default_nonce_length());

    std::string checksumChoice = checksumMatch[checksum - 1];
    if (checksumChoice == "Skein-512")
        checksumChoice = "Skein-512(256, " + std::string(uuid.begin(), uuid.end()) + ")";

    std::unique_ptr<Botan::HashFunction> hash3(Botan::HashFunction::create(checksumChoice));

    std::string hashChoice = hashMatch[hash - 1];
    if (hashChoice != "No hashing, only derivation") {
        std::unique_ptr<Botan::PasswordHashFamily> pfHash = Botan::PasswordHashFamily::create(hashChoice);
        std::unique_ptr<Botan::PasswordHash> pHash;
        if (hashChoice == "Argon2id") {
            pHash = pfHash->from_params(1000, 640000);
        } else {
            pHash = pfHash->default_params();
        }
        for (int i = 0; i < hashIters; ++i) {
            Botan::secure_vector<uint8_t> ptr(1024);
            pHash->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), uuid.data(), int(uuidLen[0]));
            password = std::string(ptr.begin(), ptr.end());
        }
    }

    Botan::secure_vector<uint8_t> ptr(32);
    std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(" + checksumChoice + ")")->default_params();

    ph->derive_key(ptr.data(), ptr.size(), password.c_str(), password.size(), iv.data(), iv.size());
    enc->set_key(ptr);

    Botan::secure_vector<uint8_t> pt(data.data(), data.data() + data.length());
    std::unique_ptr<Botan::Compression_Algorithm> ptComp = Botan::Compression_Algorithm::create("gzip");

    ptComp->start();
    ptComp->finish(pt);

    enc->start(iv);
    enc->finish(pt);

    std::string pts(pt.begin(), pt.end());

    writeData += uuidLen + std::string(uuid.begin(), uuid.end()) + nameLen + name + descLen + desc + pts;

    std::cout << writeData << std::endl;
    std::cout << writeData.size() << std::endl;
    pdb << writeData;
}

bool convert(std::string path, std::string password, std::string fileData) {
    std::ifstream pd(path, std::ios_base::binary | std::ios_base::out);
    std::string iv;
    std::getline(pd, iv);
    std::vector<uint8_t> ivd;
    try {
        ivd = Botan::hex_decode(iv);
    } catch (...) {
        return false;
    }

    if (ivd.size() != 12) return false;

    showMessage("This database may be for an older version of passman++. I will try to convert it now.");

    int uuidLen = randombytes_uniform(80);
    Botan::AutoSeeded_RNG rng;
    Botan::secure_vector<uint8_t> uuid = rng.random_vec(uuidLen);

    std::string name = split(std::string(basename(path.c_str())), '.')[0];

    encryptData(path, 1, 1, 1, 8, false, 1, uuid, name, "Converted from old database format.", fileData, password);
    return true;
}

std::vector<std::string> showErr(std::string msg) {
    displayErr("Error: database file is corrupt or invalid.\nDetails: " + msg);
    return {};
}

std::vector<std::string> parseFile(std::string path) {
    std::ifstream pd(path, std::ios_base::binary | std::ios_base::out);
    char readData[65536], len[2];
    pd.read(readData, 6);
    if (readData != "PD++" + std::to_string(char(0x11U)) + std::to_string(char(0x11U))) {
        //showErr("Invalid magic number \"" + std::string(readData) + "\".");
        return {"CONVERT"};
    }

    pd.read(readData, 1);
    if (readData != std::to_string(0x1DU)) {
        return showErr("No group separator byte after valid magic number.");
    }

    pd.read(readData, 1);
    if (int(readData[0]) > MAX_SUPPORTED_VERSION_NUMBER) {
        return showErr("Invalid version number.");
    }
    int dbVer = readData[0];

    pd.read(readData, 1);
    try {
        checksumMatch.at(int(readData[0]));
    }  catch (...) {
        return showErr("Invalid checksum option.");
    }
    int checksum = readData[0];

    pd.read(readData, 1);
    try {
        derivMatch.at(int(readData[0]));
    }  catch (...) {
        return showErr("Invalid derivation option.");
    }
    int deriv = readData[0];

    pd.read(readData, 1);
    try {
        hashMatch.at(int(readData[0]));
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
    if (readData != std::to_string(0x1DU)) {
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
    std::string sData(readData);
    Botan::secure_vector<uint8_t> uData(sData.begin(), sData.end());
    decomp->start();
    decomp->finish(uData);

    std::string name(uData.begin(), uData.end());

    pd.read(len, 2);

    pd.read(readData, toShort(len));

    std::string sDataD(readData);
    Botan::secure_vector<uint8_t> uDataD(sDataD.begin(), sDataD.end());
    decomp->start();
    decomp->finish(uDataD);

    std::string desc(uDataD.begin(), uDataD.end());

    Botan::secure_vector<uint8_t> aData(std::istreambuf_iterator<char>{pd}, {});
    decomp->start();
    decomp->finish(aData);

    std::string data(aData.begin(), aData.end());

    //std::vector<std::string> retVec;
    std::vector<std::string> retVec = {std::to_string(dbVer), std::to_string(checksum), std::to_string(deriv), std::to_string(hash), std::to_string(hashIters), std::to_string(keyFile), std::to_string(encryption), iv, uuid, name, desc, data};
    return retVec;
}
