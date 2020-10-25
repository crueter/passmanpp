#include "db.h"

unsigned long getPassLength() {
    unsigned long length = 0;
    while(1) {
        std::string slen;
        std::cout << "How long should this be? Must be at least 8: ";
        std::stringstream ss;
        std::getline(std::cin, slen);
        ss.str(slen);

        if (ss >> length) {
            if (length < 8) {
                std::cout << "Your password must be at least 8 characters. Anything less is prone to bruteforcing, and 8 is generally the minimum password length for sites." << std::endl;
                continue;
            }
            break;
        }
        std::cout << "Please use a number." << std::endl;
    }
    return length;
}

std::string genPass() {
    std::vector<std::string> ambig = {"lI", "1l", "2Z"};
    std::string passw, csChoice;
    int csInd;
    unsigned long length = getPassLength();
    std::cout << "Okay, generating a random password." << std::endl;
    std::string charset = "!#$%&'()*+,-./23456789:;<=>?@ABCDEFGHJKLMNOPRSTUVWXYZ[]^_abcdefghijkmnopqrstuvwxyz{|}~";
    passw.append(charset, rand() % charset.length(), 1);
    for (unsigned long i = 1; i < length; ++i) {
        csInd = rand() % charset.length();
        while (charset[csInd] == passw[i - 1])
            csInd = rand() % charset.length();
        passw.append(charset, csInd, 1);
    }
    return passw;
}
bool exists(sqlite3* db, std::string cmd) {
    int ar;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    ar = sqlite3_step(stmt);
    return (ar == 100);
}

std::string getpass(std::string prompt)
{
    std::cout << prompt;
    setEcho(false);
    std::string buff;
    std::getline(std::cin, buff);
    setEcho(true);
    std::cout << std::endl;

    return buff;
}

std::string addPass() {
    std::string choice, passw;
    while (1) {
        std::cout << "Do you want to input your own password? ";
        std::getline(std::cin, choice);
        if (choice == "yes") {
            while(1) {
                passw = getpass("Please enter a password. This must be unique and at least 8 characters: ");
                if (passw.length() < 8) {
                    std::cout << "Your password must be at least 8 characters. Anything less is prone to bruteforcing, and 8 characters is generally the minimum password length for sites." << std::endl;
                    continue;
                }
                if (exists(db, "SELECT * FROM data WHERE password=\"" + passw + "\"")) {
                    std::cout << "This password has already been used. DO NOT REUSE PASSWORDS! If somebody gets your password on one account, and you have the same password everywhere, all of your accounts could be compromised and sensitive info could be leaked!" << std::endl;
                    continue;
                }
                break;
            }
            break;
        } else if (choice == "no") {
            passw = genPass();
            break;
        } else std::cout << "Please input yes or no." << std::endl;
    }
    return passw;
}

std::string getNotes() {
    std::string notes, line;
    while (std::getline(std::cin, line)) {
        if (line.empty())
            break;
        notes += "\"" + line + "\"\n";
    }
    std::string trnotes = trim(notes);
    replaceAll(trnotes, "\n", " || char(10) || ");
    return trnotes;
}

void encrypt(std::string akey, Botan::secure_vector<uint8_t> iv) {
    std::ofstream pdbp(path, std::ios_base::binary);
    Botan::SHA_256 sha;
    Botan::secure_vector<uint8_t> pr = sha.process(akey);
    std::string skey(pr.begin(), pr.end());

    std::unique_ptr<Botan::Cipher_Mode> enc = Botan::Cipher_Mode::create("AES-256/GCM", Botan::ENCRYPTION);

    enc->set_key(pr);
    saveSt();

    Botan::secure_vector<uint8_t> pt(stList.data(), stList.data() + stList.length());
    enc->start(iv);
    enc->finish(pt);
    std::string pts(pt.begin(), pt.end());

    std::string towrite = Botan::hex_encode(iv) + "\n" + pts;
    pdbp << towrite;
    pdbp.close();
}

std::vector<std::string> getmpass(std::string txt) {
    std::ifstream pdpp(path, std::ios_base::binary);
    std::string ivh, line, mpass;
    std::getline(pdpp, ivh);
    std::vector<uint8_t> ivc = Botan::hex_decode(ivh);
    std::string iv(ivc.begin(), ivc.end());
    std::string r(std::istreambuf_iterator<char>{pdpp}, {});
    Botan::secure_vector<uint8_t> rp;
    
    while(1) {
        rp = Botan::secure_vector<uint8_t>(r.begin(), r.end());
        mpass = getpass("Please enter your master password" + txt + ": ");
        Botan::SHA_256 sha;
        std::unique_ptr<Botan::Cipher_Mode> decr = Botan::Cipher_Mode::create("AES-256/GCM", Botan::DECRYPTION);
        Botan::secure_vector<uint8_t> pkey = sha.process(mpass);

        decr->set_key(pkey);
        decr->start(ivc);
        try {
            decr->finish(rp);
        } catch (...) {
            std::cout << "Wrong password, please try again." << std::endl;
            continue;
        }
        break;
    }
    std::string rpr(rp.begin(), rp.end());
    return {rpr, mpass, iv};
}
