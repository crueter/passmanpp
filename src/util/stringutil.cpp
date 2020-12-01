#include "../constants.h"

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty()) {
        return;
    }
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

bool find(std::vector<std::string> set, char* cInd) {
    return std::find(set.begin(), set.end(), cInd) != set.end();
}

std::string trim(const std::string& line) {
    std::size_t start = line.find_first_not_of(whiteSpace);
    std::size_t end = line.find_last_not_of(whiteSpace);
    return start == end ? std::string() : line.substr(start, end - start + 1);
}

std::string trimNull(const std::string& line) {
    std::size_t start = line.find_first_not_of("\u0000");
    std::size_t end = line.find_last_not_of("\u0000");
    return start == end ? std::string() : line.substr(start, end - start + 1);
}

std::string join(const std::vector<std::string> &elements, const std::string &delim) {
    if (!elements.empty())
    {
        std::stringstream ss;
        auto it = elements.cbegin();
        while (true)
        {
            ss << *it++;
            if (it != elements.cend()) {
                ss << delim;
            }
            else {
                return ss.str();
            }
        }
    }
    return "";
}

std::vector<std::string> split(std::string text, char delim) {
    std::string tmp;
    std::vector<std::string> stk;
    std::stringstream ss(text);
    while(std::getline(ss, tmp, delim)) {
        stk.push_back(tmp);
    }
    return stk;
}

std::string atos(int asciiVal) {
    return std::string(reinterpret_cast<char*>(&asciiVal));
}

Botan::secure_vector<uint8_t> toVec(std::string str) {
    return Botan::secure_vector<uint8_t>(str.begin(), str.end());
}

Botan::secure_vector<uint8_t> toVec(char *str, int length) {
    return Botan::secure_vector<uint8_t>(str, str + length);
}

char *toChar(Botan::secure_vector<uint8_t> vec) {
    return reinterpret_cast<char *>(vec.data());
}

std::string toStr(Botan::secure_vector<uint8_t> vec) {
    return std::string(vec.begin(), vec.end());
}
