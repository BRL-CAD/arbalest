cmake_minimum_required(VERSION 3.1.0)

project(arbalest)

set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

set(arbalest_Include_Dirs
        "src/windows"
        "include"
        "dependancies/glm/"
        ${BRLCAD_INCLUDE_DIRS})

set(arbalest_Sources

        src/experiment/

        src/windows/MainWindow.ui
        src/windows/MainWindow.cpp
        src/main.cpp
        src/Document.cpp
        src/ObjectsTreeView.cpp
        src/display/VectorListRenderer.cpp
        src/display/Camera.cpp
        src/display/Display.cpp
        src/display/GraphicsViewOpenGL.cpp
        src/display/GridRenderer.cpp
        src/GeometryOperationsManager.cpp
        src/ArbalestGlobals.cpp)

set(arbalest_Link_Libraries
        coreinterface
        Qt5::Widgets
        Qt5::OpenGL
        Qt5::Gui
        ${BRLCAD_NMG_LIBRARY}
        ${BRLCAD_WDB_LIBRARY}
        ${BRLCAD_RT_LIBRARY}
        ${BRLCAD_BN_LIBRARY}
        ${BRLCAD_BU_LIBRARY}
        OpenGL::GL
        OpenGL::GLU)

find_package(Qt5 COMPONENTS Widgets Gui OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

file(GLOB arbalest_HEADERS_TO_MOC ./include/*)
qt5_wrap_cpp(arbalest_PROCESSED_MOCS  ${arbalest_HEADERS_TO_MOC} TARGET arbalest OPTIONS --no-notes)

include_directories(${arbalest_Include_Dirs})
add_executable(arbalest ${arbalest_Sources} ${arbalest_PROCESSED_MOCS})
target_link_libraries(arbalest ${arbalest_Link_Libraries})
