cmake_minimum_required(VERSION 3.1.0)

project(arbalest)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(arbalest_Include_Dirs
        "mainwindow"
        "include"
        ${BRLCAD_INCLUDE_DIRS})

set(arbalest_Sources
        mainwindow/MainWindow.ui
        mainwindow/MainWindow.cpp
        main.cpp
        display/GraphicsViewOpenGL.cpp
        GeometryOperationsManager.cpp
        ArbalestGlobals.cpp
        DatabaseManager.cpp)

set(arbalest_Link_Libraries
        coreinterface
        Qt5::Widgets
        Qt5::OpenGL
        ${BRLCAD_NMG_LIBRARY}
        ${BRLCAD_WDB_LIBRARY}
        ${BRLCAD_RT_LIBRARY}
        ${BRLCAD_BN_LIBRARY}
        ${BRLCAD_BU_LIBRARY})

find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

file(GLOB arbalest_HEADERS_TO_MOC ./include/*)
qt5_wrap_cpp(arbalest_PROCESSED_MOCS  ${arbalest_HEADERS_TO_MOC} TARGET arbalest OPTIONS --no-notes)

include_directories(${arbalest_Include_Dirs})
add_executable(arbalest ${arbalest_Sources} ${arbalest_PROCESSED_MOCS})
target_link_libraries(arbalest ${arbalest_Link_Libraries})
