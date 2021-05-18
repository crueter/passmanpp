cmake_minimum_required(VERSION 3.5)

project(passman++ LANGUAGES CXX VERSION 2.1.1)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOMOC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(GNUInstallDirs)

set(BIN_INSTALL_DIR "${CMAKE_INSTALL_BINDIR}")
set(DATA_INSTALL_DIR "${CMAKE_INSTALL_DATAROOTDIR}")

add_executable(passman++
        src/main.cpp
        src/database.cpp

        src/gui/mainwindow.cpp
        src/gui/password_generator_dialog.cpp

        src/gui/password_widget.cpp
        src/gui/config_widget.cpp
        src/gui/entry_edit_widget.cpp
        src/gui/welcome_widget.cpp
        src/gui/database_main_widget.cpp

        src/actions/password_generator_action.cpp
        src/actions/password_visible_action.cpp

        src/breeze.qrc

#        ../libpassman/src/data_stream.cpp
#        ../libpassman/src/extra.cpp
#        ../libpassman/src/field.cpp
#        ../libpassman/src/kdf.cpp
#        ../libpassman/src/pdpp_database.cpp
#        ../libpassman/src/pdpp_entry.cpp
#        ../libpassman/src/vector_union.cpp
)

find_package(Qt6 COMPONENTS Widgets REQUIRED)
find_package(Qt6 COMPONENTS Sql REQUIRED)

include(FindPkgConfig)

pkg_check_modules(PASSMAN REQUIRED passman)
pkg_check_modules(BOTAN2 REQUIRED botan-2)

target_include_directories(passman++ PUBLIC /usr/include/botan-2)
# target_include_directories(passman++ PRIVATE ../libpassman/include)

target_link_libraries(passman++ PRIVATE
        Qt::Widgets
        Qt::Gui
        Qt::Sql
        botan-2
        passman
)

target_compile_options(passman++ PUBLIC -O2 -march=native -Wall -Wextra -Wshadow -Wnon-virtual-dtor -Wunused -Wold-style-cast -Wconversion -Wmisleading-indentation -Wduplicated-cond -Wduplicated-branches -pedantic -Wuseless-cast)

install(TARGETS passman++
    BUNDLE DESTINATION . COMPONENT Runtime
    RUNTIME DESTINATION ${BIN_INSTALL_DIR} COMPONENT Runtime)

add_subdirectory(share)
