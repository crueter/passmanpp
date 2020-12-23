# Generated from passman.pro.

#####################################################################
## passman Binary:
#####################################################################
cmake_minimum_required(VERSION 3.5)

project(passman LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(passman
        src/constants.h
        src/database.cpp src/database.h
        src/main.cpp
        src/util/extra.cpp src/util/extra.h
        src/util/generators.cpp src/util/generators.h
        src/util/sql.cpp src/util/sql.h
        src/entry.cpp src/entry.h
        src/field.cpp src/field.h
)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Sql REQUIRED)

target_include_directories(passman PUBLIC /usr/include/botan-2 src src/handlers src/util)

target_link_libraries(passman PRIVATE
	Qt::Widgets
        Qt::Gui
        Qt::Sql
        sqlite3
        botan-2
        sodium
        stdc++fs
)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2 -march=native")
