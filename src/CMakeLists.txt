cmake_minimum_required(VERSION 3.5)

project(passman LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(passman
        main.cpp
        database.cpp database.hpp
        entry.cpp entry.hpp
        field.hpp

        constants.hpp

        util/extra.hpp
        util/vector_union.hpp
        util/data_stream.hpp

        gui/database_window.cpp gui/database_window.hpp
        gui/password_dialog.cpp gui/password_dialog.hpp
        gui/config_dialog.cpp gui/config_dialog.hpp
        gui/entry_edit_dialog.cpp gui/entry_edit_dialog.hpp
        gui/welcome_dialog.cpp gui/welcome_dialog.hpp
        gui/random_password_dialog.cpp gui/random_password_dialog.hpp

        actions/password_generator_action.cpp actions/password_generator_action.hpp
        actions/password_visible_action.cpp actions/password_visible_action.hpp

        breeze.qrc
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
)

target_compile_options(passman PUBLIC -O2 -march=native -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -Wold-style-cast -Wconversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -pedantic -Wuseless-cast)
