cmake_minimum_required(VERSION 3.16)

project(file-explorer VERSION 0.1 LANGUAGES CXX)

# Set build type to Release
set(CMAKE_BUILD_TYPE Release)

# Enable additional Qt components
set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

unset(CMAKE_PREFIX_PATH)

find_package(OpenGL REQUIRED)
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS
    Widgets
    Core
    Gui
)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS
    Widgets
    Core
    Gui
)

find_package(QuaZip-Qt6 REQUIRED)
include_directories(${QuaZip-Qt6_INCLUDE_DIRS})

set(PROJECT_SOURCES
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    propertiesdialog.h
    propertiesdialog.cpp
)

add_executable(file-explorer
    ${PROJECT_SOURCES}
)

target_link_libraries(file-explorer
    PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    QuaZip::QuaZip
    ZLIB::ZLIB
    OpenGL::GL
)

set_target_properties(file-explorer PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/release
    RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/release
)

# Installation
include(GNUInstallDirs)
install(TARGETS file-explorer
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
