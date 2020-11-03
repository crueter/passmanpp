#include <sstream>
#include <ios>
#include <iterator>
#include <algorithm>
#include <sodium.h>

#include "db.h"

uint32_t randomChar() {
    return 0x21U + randombytes_uniform(0x7EU - 0x20U);
}

bool exists(std::string cmd) {
    int ar;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    ar = sqlite3_step(stmt);
    return (ar == 100);
}

std::string genPass(int length, bool capitals, bool numbers, bool symbols) {
    std::string passw, csChoice, ssInd;
    uint32_t csInd;

    std::cout << "Okay, generating a random password." << std::endl;

    std::vector<std::string> capital = {"A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};
    std::vector<std::string> number = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9"};
    std::vector<std::string> symbol = {"!", "#", "$", "%", "&", "'", "(", ")", "*", "+", ",", "-", ".", "/", ":", ";", "<", "=", ">", "?", "@", "[", "]", "^", "_", "`", "{", "|", "}", "~"};

    for (int i = 0; i < length; ++i) {
        csInd = randomChar();
        ssInd = std::to_string(csInd);
        while (1) {
            csInd = randomChar();
            ssInd = std::to_string(csInd);
            char* cInd = reinterpret_cast<char*>(&csInd);
            if ((i != 0 && ssInd == std::to_string(passw[i - 1])) || csInd == 0x22U || csInd == 0x5CU)
                continue;
            if (capitals && std::find(capital.begin(), capital.end(), cInd) != capital.end())
                continue;
            if (numbers && std::find(number.begin(), number.end(), cInd) != number.end())
                continue;
            if (symbols && std::find(symbol.begin(), symbol.end(), cInd) != symbol.end())
                continue;
            break;
        }
        passw.append(reinterpret_cast<char*>(&csInd));
    }
    return passw;
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

std::string input(std::string prompt) {
    std::string inp;
    std::cout << prompt;
    std::getline(std::cin, inp);
    return inp;
}

void encrypt(std::string akey, Botan::secure_vector<uint8_t> iv, std::string apath) {
    std::ofstream pdbp(apath, std::ios_base::binary);
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
