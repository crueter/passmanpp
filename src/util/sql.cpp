#include "sql.h"

QSqlDatabase db;

void dbInit() {
    db = QSqlDatabase::addDatabase("QSQLITE", ":memory:");
    bool dbOk = db.open();
    if (!dbOk) {
        displayErr("Error while opening database: " + db.lastError().text() + QString::fromStdString("\nPlease open an issue on " + github + " for help with this."));
        exit(1);
    }
}


QList<QSqlQuery> selectAll() {
    QSqlQuery selQuery(db);
    selQuery.exec("SELECT tbl_name FROM 'sqlite_master' WHERE type='table'");
    QList<QSqlQuery> queries;

    while (selQuery.next()) {
        QString val = selQuery.value(0).toString();

        if (val.isEmpty()) {
            return {};
        }
        QString lSt = "SELECT * FROM '" + val + "'";

        QSqlQuery query(db);
        bool ok = query.exec(lSt);

        if (!ok) {
            displayErr("Warning: SQL execution error: " + query.lastError().text() + "\nQuery: " + lSt);
        }
        queries.push_back(query);
    }
    selQuery.finish();

    return queries;
}

void execAll(const QString &stmt) {
    for (const QString &st : stmt.split("\n")) {
        if (st.isEmpty()) {
            continue;
        }

        QSqlQuery finalQ(db);
        bool ok = finalQ.exec(st);

        if (!ok) {
            displayErr("Warning: SQL execution error: " + finalQ.lastError().text() + "\nQuery: " + st);
        }

        finalQ.finish();
    }
}

bool exists(const QString &field, const QString &value) {
    for (QSqlQuery q : selectAll()) {
        while (q.next()) {
            if (q.value(q.record().indexOf(field)) == value) {
                return true;
            }
        }
    }
    return false;
}
