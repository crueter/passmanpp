#include "sql.h"

QSqlDatabase db;

void dbInit() {
    db = QSqlDatabase::addDatabase("QSQLITE", ":memory:");
    bool dbOk = db.open();
    if (!dbOk) {
        std::cerr << "Error while opening database: " << db.lastError().text().toStdString() << std::endl << "Please open an issue on https://github.com/binex-dsk/passmanpp for help with this.";
        exit(1);
    }
}


QList<QSqlQuery> selectAll() {
    QSqlQuery selQuery(db);
    selQuery.exec("SELECT tbl_name FROM 'sqlite_master' WHERE type='table' ORDER BY tbl_name");
    QList<QSqlQuery> queries;

    while (selQuery.next()) {
        QString val = selQuery.value(0).toString();

        if (val == "") {
            return {};
        }
        QString lSt = "SELECT * FROM '" + val + "'";

        QSqlQuery query(db);
        bool ok = query.exec(lSt);

        if (!ok) {
            qDebug() << "Warning: SQL execution error:" << query.lastError();
            qDebug() << "Query:" << lSt;
        }
        queries.push_back(query);
    }
    selQuery.finish();

    return queries;
}

void execAll(const QString &stmt) {
    for (const QString &st : stmt.split("\n")) {
        if (st == "") {
            continue;
        }

        QSqlQuery finalQ(db);
        bool ok = finalQ.exec(st);

        if (!ok) {
            qDebug() << "Warning: SQL execution error:" << finalQ.lastError();
            qDebug() << "Query:" << st;
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
