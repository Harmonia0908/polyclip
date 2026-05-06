cmake_minimum_required(VERSION 3.10)

project(PolygonClipDemo LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt5 REQUIRED COMPONENTS Widgets)

add_executable(PolygonClipDemo
    src/main.cpp
    src/MainWindow.h
    src/MainWindow.cpp
    src/CanvasWidget.h
    src/CanvasWidget.cpp
    src/Geometry.h
    src/Geometry.cpp
    src/WeilerAtherton.h
    src/WeilerAtherton.cpp
)

target_link_libraries(PolygonClipDemo Qt5::Widgets)
