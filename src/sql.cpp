#include "sql.h"

std::vector<std::string> names;
std::string glob_stList;
std::string unionSt;
std::vector<std::string> unionSts;

sqlite3* db;
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
    glob_stList += "\nINSERT INTO data (" + colsd + ") VALUES (" + datad + ")";
    return 0;
}

int exec(std::string cmd, Database tdb, bool save, int (*callback)(void*, int, char**, char**)) {
    char* err = 0;
    int arc = sqlite3_exec(db, cmd.c_str(), callback, 0, &err);
    if (arc != SQLITE_OK && std::string(err) != "query aborted") { // sort of band-aid fix right now; I'll fix later
        std::cerr << "Warning: SQL execution error: " << std::string(err) << std::endl;
    }
    if (save) {
        saveSt(tdb);
    }
    return arc;
}

int _getUnion(void *, int, char **data, char **) {
    unionSts.push_back(std::string("SELECT * FROM ") + data[0] + " ORDER BY name");
    return 0;
}

int _createSt(void *, int, char **data, char **) {
    glob_stList += data[0];
    return 0;
}

void saveSt(Database tdb) {
    glob_stList = "";
    unionSts = {};

    exec("SELECT name FROM sqlite_master WHERE type='table'", tdb, false, _getUnion);
    unionSt = join(unionSts, " UNION ");

    exec("SELECT sql FROM sqlite_master WHERE type='table'", tdb, false, _createSt);

    exec(unionSt, tdb, false, _saveSt);
    tdb.stList = glob_stList;
}

bool exists(std::string cmd) {
    int ar;
    sqlite3_stmt *stmt;
    sqlite3_prepare_v2(db, cmd.c_str(), -1, &stmt, NULL);
    ar = sqlite3_step(stmt);
    return (ar == 100);
}

std::vector<std::string> getNames(Database tdb) {
    names = {};
    exec("SELECT name FROM data ORDER BY name", tdb, false, _getNames);
    return names;
}
