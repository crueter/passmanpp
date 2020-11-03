TEMPLATE = app
TARGET = passman

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.cpp db.cpp sql.cpp stringutil.cpp manager.cpp

QMAKE_CXXFLAGS += -O2 -fpermissive -std=c++20 -Wunused-parameter

LIBS += -lbotan-2 -lsqlite3 -lstdc++fs -lsodium

HEADERS += db.h sql.h file_handler.h stringutil.h manager.h \
    entry_handler.h

INCLUDEPATH += "/usr/include/botan-2/"
