#include "sql.h"

std::vector<std::string> names;
std::ofstream pdb;
std::string stList;

sqlite3* db;
bool modified;
int _rc = sqlite3_open(":memory:", &db);

int _getNames(void *, int, char **data, char **) {
    names.push_back(data[0]);
    return 0;
}

int _saveSt(void *, int count, char **data, char **cols) {
    std::string datad, colsd;
    for (int i = 0; i < count; ++i) {
        std::string di = data[i];
        replaceAll(di, "\n", " || char(10) || ");
        datad += std::string(i == 0 ? "" : ", ") + "\"" + di + "\"";
        colsd += std::string(i == 0 ? "" : ", ") + "\"" + cols[i] + "\"";
    }
    stList += "\nINSERT INTO data (" + colsd + ") VALUES (" + datad + ")";
    return 0;
}

int exec(std::string cmd, bool save, int (*callback)(void*, int, char**, char**)) {
    char* err = 0;
    int arc = sqlite3_exec(db, cmd.c_str(), callback, 0, &err);
    if (arc != SQLITE_OK && std::string(err) != "query aborted") // sort of band-aid fix right now; I'll fix later
        std::cout << "Warning: SQL execution error: " << std::string(err) << std::endl;
    if (save) saveSt();
    return arc;
}

void saveSt() {
    stList = "CREATE TABLE data (name text, email text, url text, notes text, password text)";
    exec("SELECT * FROM data ORDER BY name", false, _saveSt);
}

std::vector<std::string> getNames() {
    names = {};
    exec("SELECT name FROM data ORDER BY name", false, _getNames);
    return names;
}
