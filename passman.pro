TEMPLATE = app
TARGET = passman

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += src/main.cpp src/util/sql.cpp src/util/stringutil.cpp src/handlers/entry_handler.cpp src/handlers/file_handler.cpp src/database.cpp src/util/generators.cpp src/util/manager.cpp

QMAKE_CXXFLAGS += -O2 -march=native

LIBS += -lbotan-2 -lsqlite3 -lstdc++fs -lsodium

HEADERS += src/util/sql.h src/util/stringutil.h src/handlers/entry_handler.h src/handlers/file_handler.h src/database.h src/util/generators.h src/util/manager.h src/constants.h

INCLUDEPATH += "/usr/include/botan-2/" "src/util/" "src/handlers"
