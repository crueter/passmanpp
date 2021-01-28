#include <QApplication>
#include <QAbstractButton>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <memory>

#include "util/extra.hpp"
#include "util/sql.hpp"
#include "gui/welcome_dialog.hpp"
#include "util/vector_union.hpp"

bool debug = false;
bool verbose = false;

int main(int argc, char** argv) {
    QApplication app (argc, argv);

    if (getenv("PASSMAN_DEBUG")) {
        std::cout << "Debug mode activated. Do NOT use this unless you are testing stuff." << std::endl;
        debug = true;
    }
    if (getenv("PASSMAN_VERBOSE")) {
        std::cout << "Verbose mode activated. Do NOT use this unless you are testing stuff." << std::endl;
        verbose = true;
    }

    dbInit();
    auto database = std::make_shared<Database>();
    QString path;

    auto create = [&](QString spath = "") mutable {
            if (spath.isEmpty()) {
                path = newLoc();
                if (path.isEmpty()) {
                    exit(1);
                }
            } else {
                path = spath;
            }
            database->path = path;
            bool cr = database->config(true);
            if (!cr) {
                exit(1);
            }
            return true;
    };

    auto open = [&](QString spath) {
        database->path = spath;

        if (!database->open()) {
            exit(1);
        }
    };

    bool createNew = false;

    if (argc <= 1) {
        auto di = std::make_unique<WelcomeDialog>(database);
        di->setup();

        if (di->show() == QDialog::Rejected) {
            return 1;
        }
    } else {
        for (const int i : range(1, argc)) {
            const QString arg(argv[i]);

            if (arg == "new") {
                createNew = true;
            } else if (arg == "help") {
                qDebug() << usage.data();
                return 1;
            } else if (arg == "info") {
                qDebug() << info.data();
                return 1;
            } else if (createNew) {
                path = arg;
            } else {
                open(arg);
            }
        }
    }

    if (createNew) {
        create(path);
    }

    database->edit();
    database->save();

    return 0;
}
