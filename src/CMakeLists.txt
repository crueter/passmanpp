cmake_minimum_required(VERSION 3.5)

project(passman LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(passman
        main.cpp
        database.cpp database.h
        entry.cpp entry.h
        field.cpp field.h

        constants.h

        util/extra.cpp util/extra.h
        util/sql.cpp util/sql.h
        util/vector_union.cpp util/vector_union.h
        util/data_stream.cpp util/data_stream.h

        gui/database_window.cpp gui/database_window.h
        gui/password_dialog.cpp gui/password_dialog.h
        gui/config_dialog.cpp gui/config_dialog.h
        gui/entry_edit_dialog.cpp gui/entry_edit_dialog.h
        gui/welcome_dialog.cpp gui/welcome_dialog.h
        gui/random_password_dialog.cpp gui/random_password_dialog.h

        actions/password_generator_action.cpp actions/password_generator_action.h
        actions/password_visible_action.cpp actions/password_visible_action.h
)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Sql REQUIRED)

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

target_compile_options(passman PUBLIC -O2 -march=native)
