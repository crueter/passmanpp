TEMPLATE = app
TARGET = passman

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.cpp sql.cpp stringutil.cpp entry_handler.cpp file_handler.cpp \
    database.cpp \
    generators.cpp \
    manager.cpp

QMAKE_CXXFLAGS += -O2 -march=native -std=c++17

LIBS += -lbotan-2 -lsqlite3 -lstdc++fs -lsodium

HEADERS += sql.h file_handler.h stringutil.h entry_handler.h constants.h \
    database.h \
    generators.h \
    manager.h

INCLUDEPATH += "/usr/include/botan-2/"
