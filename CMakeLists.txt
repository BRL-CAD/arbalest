cmake_minimum_required(VERSION 3.1.0)

project(arbalest)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)



include_directories("mainwindow" "include"  ${CMAKE_BINARY_DIR} ${BRLCAD_INCLUDE_DIRS} ${BRLCAD_INCLUDE_DIR} ${RT3_SOURCE_DIR}/include/brlcad)

find_package(Qt5 COMPONENTS Widgets OpenGL REQUIRED)

add_executable(arbalest
        mainwindow/MainWindow.ui
        mainwindow/MainWindow.cpp
        main.cpp
        display/GraphicsViewOpenGL.cpp
        Globals.cpp
        DatabaseManager.cpp
        )


qt5_use_modules(arbalest OpenGL)

target_link_libraries(arbalest coreinterface  ${BRLCAD_NMG_LIBRARY} ${BRLCAD_WDB_LIBRARY} ${BRLCAD_RT_LIBRARY} ${BRLCAD_BN_LIBRARY} ${BRLCAD_BU_LIBRARY} ${OPENGL_LIBRARIES})

