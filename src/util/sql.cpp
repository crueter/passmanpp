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
        }
        queries.push_back(query);
    }
    selQuery.finish();

    return queries;
}

QString getCreate(QString name, QStringList names, QList<QMetaType> types, QVariantList values) {
    QString saveStr = "CREATE TABLE '" + name + "' (";
    for (int i = 0; i < names.size(); ++i) {
        QList<QMetaType> varTypes = {QMetaType(QMetaType::QString), QMetaType(QMetaType::QString), QMetaType(QMetaType::QString)};
        QList<QString> sqlTypes = {"text", "integer", "real"};
        int index = varTypes.indexOf(types[i]);
        saveStr += names[i].replace("\"", "'") + " " + sqlTypes[index];
        if (i != names.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ")\nINSERT INTO '" + name.replace("\"", "'") + "' (";
        }
    }

    for (int i = 0; i < names.size(); ++i) {
        saveStr += names[i].replace("\"", "'");
        if (i != names.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ") VALUES (";
        }
    }

    for (int i = 0; i < values.size(); ++i) {
        QVariant val = values[i];
        QString quote = "";
        if (types[i] == QMetaType(QMetaType::QString)) {
            quote = "\"";
        }
        saveStr += quote + val.toString().replace("\"", "'") + quote;
        if (i != values.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ")\n";
        }
    }

    return saveStr;
}

void execAll(QString stmt) {
    for (QString st : stmt.split("\n")) {
        if (st == "") {
            continue;
        }
        QSqlQuery finalQ(db);
        bool ok = finalQ.exec(st);

        if (!ok) {
            qDebug() << "Warning: SQL execution error:" << finalQ.lastError();
        }

        finalQ.finish();
    }
    saveSt();
}

void execAll(std::string stmt) {
    execAll(QString::fromStdString(stmt));
}

std::string saveSt(bool exec) {
    QString execSt = "";

    for (QSqlQuery q : selectAll()) {
        while (q.next()) {
            QSqlRecord rec = q.record();

            QString name = q.value(0).toString();

            QStringList names;
            QList<QMetaType> types;
            QVariantList values;
            for (int i = 0; i < rec.count(); ++i) {
                QSqlField field = rec.field(i);
                names.push_back(field.name());

                QMetaType fType = field.metaType();
                types.push_back(fType);

                QVariant fData = field.value();
                values.push_back(fData);
            }
            execSt += getCreate(name, names, types, values);
        }
        q.finish();
    }

    if (exec) {
        QSqlQuery delQuery(db);
        delQuery.exec("SELECT 'DROP TABLE \"' || name || '\"' FROM sqlite_master WHERE type = 'table'");
        QString delSt;

        while (delQuery.next()) {
            delSt += delQuery.value(0).toString() + "\n";
        }
        delQuery.finish();
        db.exec(delSt);

        execAll(execSt);
    }
    return execSt.toStdString();
}

bool exists(QString field, QString value) {
    for (QSqlQuery q : selectAll()) {
        while (q.next()) {
            if (q.value(q.record().indexOf(field)) == value) {
                return true;
            }
        }
    }
    return false;
}
