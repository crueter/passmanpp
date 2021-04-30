#include <QApplication>
#include <QCommandLineParser>
#include <QSqlError>
#include <QFile>
#include <QFileDialog>

#include <botan/bigint.h>
#include <passman/constants.hpp>

#include "gui/welcome_widget.hpp"
#include "gui/password_widget.hpp"
#include "passman_constants.hpp"

// TODO: constexpr, noexcept

int main(int argc, char** argv) {
    QApplication app (argc, argv);
    QApplication::setApplicationName(QObject::tr("passman++"));
    QApplication::setApplicationVersion(passman::tr(Constants::passmanVersion));

    QCommandLineParser parser;
    parser.setApplicationDescription(QObject::tr("A simple, minimal, and just as powerful and secure password manager."));
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument(QObject::tr("path"), QObject::tr("Path to a database file, or a path to where you want to create a new database."));

    QCommandLineOption newOption(QStringList() << "n" << "new", QObject::tr("Create a new database."));
    QCommandLineOption debugOption(QStringList() << "d" << "debug", QObject::tr("Activate debug mode."));
    QCommandLineOption verboseOption(QStringList() << "V" << "verbose", QObject::tr("Activate verbose mode."));
    QCommandLineOption themeOption(QStringList() << "t" << "theme", QObject::tr("Specify light or dark mode"), QObject::tr("mode"), QObject::tr("dark"));

    parser.addOptions({newOption, debugOption, verboseOption, themeOption});

    parser.process(app);

    const QStringList args = parser.positionalArguments();

    QString theme = parser.value(themeOption);
    if (!QStringList{QObject::tr("dark"), QObject::tr("light")}.contains(parser.value(themeOption))) {
        std::cerr << "Invalid theme option. Must be one of: light, dark\n";
        std::cerr << "Defaulting to dark theme." << std::endl;
        theme = "dark";
    }

    passman::db.open();

    QFile file(":/" + theme + ".qss");
    file.open(QFile::ReadOnly | QFile::Text);
    QTextStream stream(&file);
    app.setStyleSheet(stream.readAll());

    app.setProperty("theme", theme);

    QString path{};
    if (args.length() > 0) {
        path = args.at(0);
    }

    MainWindow *mainWindow = new MainWindow;

    Database *database = new Database(mainWindow);

    if (parser.isSet(newOption)) {
        createDatabase(database, path);
    } else if (!path.isEmpty()) {
        if (!openDb(database, path)) {
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
