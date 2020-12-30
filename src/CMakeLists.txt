cmake_minimum_required(VERSION 3.5)

project(passman LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(passman
        constants.h
        database.cpp database.h
        main.cpp
        util/extra.cpp util/extra.h
        util/generators.cpp util/generators.h
        util/sql.cpp util/sql.h
        entry.cpp entry.h
        field.cpp field.h
)

find_package(QT NAMES Qt6 Qt5 COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Sql REQUIRED)

target_include_directories(passman PUBLIC /usr/include/botan-2)

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
