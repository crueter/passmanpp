#include "sql.h"
#include <QDebug>

QSqlDatabase db;

void dbInit() {
    db = QSqlDatabase::addDatabase("QSQLITE", ":memory:");
    bool dbOk = db.open();
    if (!dbOk) {
        std::cerr << "Error while opening database: " << db.lastError().text().toStdString() << std::endl << "Please open an issue on https://github.com/binex-dsk/passmanpp for help with this.";
        exit(1);
    }
}

QString typeConv(QVariant::Type type) {
    QVector<QVariant::Type> varTypes = {QVariant::String, QVariant::Int, QVariant::Double};
    QVector<QString> sqlTypes = {"text", "integer", "real"};
    int index = varTypes.indexOf(type);
    return sqlTypes[index];
}

QVector<QSqlQuery> selectAll() {
    QSqlQuery selQuery(db);
    selQuery.exec("SELECT name FROM sqlite_master WHERE type='table'");
    QVector<QSqlQuery> queries;

    while (selQuery.next()) {
        qDebug() << selQuery.value(0).toString();
        QString lSt = "SELECT * FROM " + selQuery.value(0).toString() + " ORDER BY name";
        QSqlQuery query(db);
        bool ok = query.exec(lSt);
        if (!ok) {
            qDebug() << query.lastError();
        }
        queries.push_back(query);
    }
    selQuery.finish();

    return queries;
}

QString getCreate(QString name, QStringList names, QList<QVariant::Type> types, QVariantList values) {
    QString saveStr = "CREATE TABLE '" + name + "' (";
    for (int i = 0; i < names.size(); ++i) {
        saveStr += names[i] + " " + typeConv(types[i]);
        if (i != names.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ")\nINSERT INTO " + name + "(";
        }
    }

    for (int i = 0; i < names.size(); ++i) {
        saveStr += names[i];
        if (i != names.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ") VALUES (";
        }
    }

    for (int i = 0; i < values.size(); ++i) {
        QVariant val = values[i];
        QString quote = "";
        if (types[i] == QVariant::String) {
            quote = "\"";
        }
        saveStr += quote + val.toString() + quote;
        if (i != values.size() - 1) {
            saveStr += ", ";
        } else {
            saveStr += ")\n";
        }
    }
    return saveStr;
}

void execAll(std::string stmt) {
    for (std::string st : split(stmt, '\n')) {
        QSqlQuery finalQ(db);
        bool ok = finalQ.exec(QString::fromStdString(st));
        if (!ok) {
            qDebug() << "Warning: SQL execution error:" << finalQ.lastError();
        }
        finalQ.finish();
    }
}

std::string saveSt(Database tdb, bool exec) {
    QVector<QSqlQuery> queries = selectAll();
    QString execSt = "";

    for (QSqlQuery q : queries) {
        while (q.next()) {
            QSqlRecord rec = q.record();
            QString name = q.value(0).toString();

            QStringList names;
            QList<QVariant::Type> types;
            QVariantList values;
            for (int i = 0; i < rec.count(); ++i) {
                QSqlField field = rec.field(i);
                names.push_back(field.name());

                QVariant::Type fType = field.type();
                types.push_back(fType);

                QVariant fData = field.value();
                values.push_back(fData);

                qDebug() << "Column" << i;
                qDebug() << "NAME:" << field.name();
                qDebug() << "VALUE:" << fData.toString();
                qDebug() << "TYPE:" << fType;
                qDebug() << "SQL TYPE:" << typeConv(fType) << Qt::endl;
            }
            execSt += getCreate(name, names, types, values);
        }
        q.finish();
    }
    tdb.stList = execSt.toStdString();
    std::cout << execSt.toStdString() << std::endl;
    std::cout << tdb.stList << std::endl;

    if (exec) {
        QSqlQuery delQuery(db);
        delQuery.exec("SELECT 'DROP TABLE \"' || name || '\"' FROM sqlite_master WHERE type = 'table'");
        QString delSt;

        while (delQuery.next()) {
            delSt += delQuery.value(0).toString() + "\n";
        }
        delQuery.finish();
        db.exec(delSt);

        execAll(execSt.toStdString());
    }
    return execSt.toStdString();
}

bool exists(QString field, QString value) {
    QVector<QSqlQuery> all = selectAll();
    for (QSqlQuery q : all) {
        if (q.value(q.record().indexOf(field)) == value) {
            return true;
        }
    }
    return false;
}
