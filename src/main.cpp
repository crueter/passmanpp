#include <QApplication>
#include <QCommandLineParser>
#include <QSqlError>

#include <botan/bigint.h>

#include "util/extra.hpp"
#include "gui/welcome_widget.hpp"

// TODO: constexpr, noexcept

QSqlDatabase db;

int main(int argc, char** argv) {
    QApplication app (argc, argv);
    QApplication::setApplicationName(tr("passman++"));
    QApplication::setApplicationVersion(tr(Constants::passmanVersion));

    QCommandLineParser parser;
    parser.setApplicationDescription(tr("A simple, minimal, and just as powerful and secure password manager."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(tr("path"), tr("Path to a database file, or a path to where you want to create a new database."));

    QCommandLineOption newOption(QStringList() << "n" << "new", tr("Create a new database."));
    QCommandLineOption debugOption(QStringList() << "d" << "debug", tr("Activate debug mode."));
    QCommandLineOption verboseOption(QStringList() << "V" << "verbose", tr("Activate verbose mode."));
    QCommandLineOption themeOption(QStringList() << "t" << "theme", tr("Specify light or dark mode"), tr("mode"), tr("dark"));

    parser.addOptions({newOption, debugOption, verboseOption, themeOption});

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    QString theme = parser.value(themeOption);
    if (!QStringList{tr("dark"), tr("light")}.contains(parser.value(themeOption))) {
        qDebug() << "Invalid theme option. Must be one of: light, dark";
        qDebug() << "Defaulting to dark theme.";
        theme = "dark";
    }

    QFile file(":/" + theme + ".qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    app.setStyleSheet(stream.readAll());

    app.setProperty("theme", theme);

    QString path{};
    if (args.length() > 0) {
        path = args.at(0);
    }

    db = QSqlDatabase::addDatabase("QSQLITE", ":memory:");

    if (!db.open()) {
        displayErr("Error while opening database: " + db.lastError().text() + tr("\nPlease open an issue on " + Constants::github + " for help with this."));
        return 1;
    }

    MainWindow *mainWindow = new MainWindow;

    Database *database = new Database(mainWindow);

    if (parser.isSet(newOption)) {
        createDatabase(database, path);
    } else if (!path.isEmpty()) {
        database->path = path;

        if (!database->open()) {
            return 1;
        }
    } else {
        WelcomeWidget *di = new WelcomeWidget(database);
        di->setup();
        di->show();
    }

    mainWindow->show();

    app.setProperty("debug", parser.isSet(debugOption));
    app.setProperty("verbose", parser.isSet(verboseOption));

    return app.exec();
}
