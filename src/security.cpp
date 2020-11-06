#include "security.h"

void encrypt(std::string akey, std::string apath) {
    std::ofstream pdbp(apath, std::ios_base::binary);

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create("AES-256/GCM", Botan::ENCRYPTION);

    saveSt();

    Botan::AutoSeeded_RNG rng;
    Botan::secure_vector<uint8_t> iv = rng.random_vec(enc->default_nonce_length());

    Botan::secure_vector<uint8_t> ptr(32);
    std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();

    ph->derive_key(ptr.data(), ptr.size(), akey.c_str(), akey.size(), iv.data(), iv.size());
    enc->set_key(ptr);

    Botan::secure_vector<uint8_t> pt(stList.data(), stList.data() + stList.length());

    enc->start(iv);
    enc->finish(pt);
    std::string pts(pt.begin(), pt.end());

    std::string towrite = Botan::hex_encode(iv) + "\n" + pts;
    pdbp << towrite;
    pdbp.close();
}

std::vector<std::string> getmpass(std::string txt, std::string path) {
    std::ifstream pdpp(path, std::ios_base::binary);
    std::string ivh, line, mpass;
    std::getline(pdpp, ivh);
    std::vector<uint8_t> ivc;

    try {
        ivc = Botan::hex_decode(ivh);
    } catch (...) {
        throw std::ios_base::failure("This is not a valid database file.");
    }

    if (ivc.size() != 12)
        throw std::range_error("Invalid IV in database header, or invalid database file.");

    std::string r(std::istreambuf_iterator<char>{pdpp}, {});
    Botan::secure_vector<uint8_t> rp;

    while(1) {
        rp = Botan::secure_vector<uint8_t>(r.begin(), r.end());
        QString pass = QInputDialog::getText(nullptr, QTranslator::tr("Enter your password"), QTranslator::tr(std::string("Please enter your master password" +  txt + ".").c_str()), QLineEdit::Password);
        mpass = pass.toStdString();

        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create("AES-256/GCM", Botan::DECRYPTION);

        Botan::secure_vector<uint8_t> ptr(32);
        std::unique_ptr<Botan::PasswordHash> ph = Botan::PasswordHashFamily::create("PBKDF2(SHA-256)")->default_params();
        ph->derive_key(ptr.data(), ptr.size(), mpass.c_str(), mpass.size(), ivc.data(), ivc.size());

        decr->set_key(ptr);

        decr->start(ivc);
        try {
            decr->finish(rp);
        } catch (std::exception& e) {
            try {
                Botan::SHA_256 sha;
                Botan::secure_vector<uint8_t> ccs = sha.process(mpass.data());
                decr->set_key(ccs);
                decr->start(ivc);

                rp = Botan::secure_vector<uint8_t>(r.begin(), r.end());
                decr->finish(rp);

                displayErr("Warning: This database file is for an older version. It will be converted to the current version's format. Please convert any of your other databases to the newer version as well.");
            }  catch (std::exception& e) {
                displayErr("Wrong password, please try again.\n" + std::string(e.what()));
                continue;
            }
        }
        break;
    }
    std::string rpr(rp.begin(), rp.end());
    return {rpr, mpass};
}
