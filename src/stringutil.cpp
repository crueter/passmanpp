#include <unistd.h>
#include <termios.h>

#include "stringutil.h"

termios tty;

void setEcho(bool echo) {
    tcgetattr(STDIN_FILENO, &tty);
    if (echo) tty.c_lflag |= ECHO;
    else tty.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &tty);
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();
    }
}

std::string trim(const std::string& line) {
    const char* whiteSpace = " \t\v\r\n";
    std::size_t start = line.find_first_not_of(whiteSpace);
    std::size_t end = line.find_last_not_of(whiteSpace);
    return start == end ? std::string() : line.substr(start, end - start + 1);
}
