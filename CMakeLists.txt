cmake_minimum_required(VERSION 3.5)

project(passman LANGUAGES CXX)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BIN_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
set(DATA_INSTALL_DIR "/usr/share")

add_executable(passman
        src/main.cpp
        src/database.cpp src/database.hpp
        src/entry.cpp src/entry.hpp
        src/field.hpp

        src/constants.hpp

        src/util/extra.hpp
        src/util/vector_union.hpp
        src/util/data_stream.hpp

        src/gui/mainwindow.cpp src/gui/mainwindow.hpp
        src/gui/password_generator_dialog.cpp src/gui/password_generator_dialog.hpp

        src/gui/base_widget.hpp
        src/gui/password_widget.cpp src/gui/password_widget.hpp
        src/gui/config_widget.cpp src/gui/config_widget.hpp
        src/gui/entry_edit_widget.cpp src/gui/entry_edit_widget.hpp
        src/gui/welcome_widget.cpp src/gui/welcome_widget.hpp
        src/gui/database_main_widget.cpp src/gui/database_main_widget.hpp

        src/actions/password_generator_action.cpp src/actions/password_generator_action.hpp
        src/actions/password_visible_action.cpp src/actions/password_visible_action.hpp

        src/breeze.qrc
)

find_library(BOTAN botan-2)
if (NOT BOTAN)
    message(FATAL_ERROR "Botan library not found. Please install through your distribution's password manager.")
endif()

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Sql REQUIRED)

target_include_directories(passman PUBLIC /usr/include/botan-2)

target_link_libraries(passman PRIVATE
        Qt::Widgets
        Qt::Gui
        Qt::Sql
        botan-2
)

target_compile_options(passman PUBLIC -O2 -march=native -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -Wold-style-cast -Wconversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -pedantic -Wuseless-cast)

install(TARGETS passman
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT Runtime)

add_subdirectory(share)
