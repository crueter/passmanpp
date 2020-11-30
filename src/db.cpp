#include <ios>
#include <iterator>
#include <algorithm>

#include "db.h"

static bool _find(std::vector<std::string> set, char* cInd) {
    return std::find(set.begin(), set.end(), cInd) != set.end();
}

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

    for (int i = 0; i < length; ++i) {
        csInd = randomChar();
        ssInd = std::to_string(csInd);
        while (1) {
            csInd = randomChar();
            ssInd = std::to_string(csInd);
            char* cInd = reinterpret_cast<char*>(&csInd);
            if ((i != 0 && ssInd == std::to_string(passw[i - 1])) || csInd == 0x22U || csInd == 0x5CU) {
                continue;
            }
            if (capitals && _find(capital, cInd)) {
                continue;
            }
            if (numbers && _find(number, cInd)) {
                continue;
            }
            if (symbols && _find(symbol, cInd)) {
                continue;
            }
            break;
        }
        passw.append(reinterpret_cast<char*>(&csInd));
    }
    return passw;
}
