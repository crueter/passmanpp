TEMPLATE = app
TARGET = passman

QT += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

SOURCES += main.cpp db.cpp sql.cpp stringutil.cpp manager.cpp entry_handler.cpp file_handler.cpp security.cpp

QMAKE_CXXFLAGS += -O2 -march=native

LIBS += -lbotan-2 -lsqlite3 -lstdc++fs -lsodium

HEADERS += db.h sql.h file_handler.h stringutil.h manager.h entry_handler.h security.h

INCLUDEPATH += "/usr/include/botan-2/"
