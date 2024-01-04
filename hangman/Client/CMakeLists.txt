cmake_minimum_required(VERSION 3.1)

project(client)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_PREFIX_PATH "/home/dawid/Qt/6.6.1/gcc_64" ${CMAKE_PREFIX_PATH}) # ścieżka do Qt, może być inna

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

find_package(QT NAMES Qt6 Qt5      COMPONENTS Core Widgets Network REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Network REQUIRED)

set(SRCS
    main.cpp
    widget.ui
    widget.cpp
    widget.h
)

add_executable(client ${SRCS})
add_compile_options(-Wall -Wextra)

target_link_libraries(client Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Network)
