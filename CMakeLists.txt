cmake_minimum_required(VERSION 3.10)

project(PolygonClipDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_GUI "Build Qt GUI" ON)

add_library(geometry_core STATIC
    src/Geometry.h
    src/Geometry.cpp
    src/WeilerAtherton.h
    src/WeilerAtherton.cpp
)

target_include_directories(geometry_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

add_executable(console_demo
    src/console_test.cpp
)

target_link_libraries(console_demo geometry_core)

if(BUILD_GUI)
    find_package(Qt5 COMPONENTS Widgets QUIET)

    if(Qt5_FOUND)
        set(CMAKE_AUTOMOC ON)
        set(CMAKE_AUTORCC ON)
        set(CMAKE_AUTOUIC ON)

        add_executable(PolygonClipDemo
            src/main.cpp
            src/MainWindow.h
            src/MainWindow.cpp
            src/CanvasWidget.h
            src/CanvasWidget.cpp
        )

        target_link_libraries(PolygonClipDemo
            geometry_core
            Qt5::Widgets
        )
    else()
        message(WARNING "Qt5 not found. GUI target will not be built. Building console_demo only.")
    endif()
endif()
