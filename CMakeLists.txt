cmake_minimum_required(VERSION 3.1.0)

project(arbalest)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(arbalest_Include_Dirs
        "src/windows"
        "include"
        "dependancies/glm/"
        ${BRLCAD_INCLUDE_DIRS}
        ${RT3_SOURCE_DIR}/include/brlcad)

set(arbalest_Sources
        src/gui/MainWindow.cpp
        src/main.cpp
        src/Document.cpp
        src/gui/ObjectTree.cpp
        src/display/GeometryRenderer.cpp
        src/display/OrthographicCamera.cpp
        src/display/PerspectiveCamera.cpp
        src/display/Display.cpp
        src/display/DisplayManager.cpp
        src/display/AxesRenderer.cpp
        src/GeometryOperationsManager.cpp
        src/Globals.cpp
        src/utils/QSSPreprocessor.cpp
        src/gui/Dockable.cpp
        src/gui/Properties.cpp
        src/gui/CollapsibleWidget.cpp)

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

IF (WIN32)
    find_library(user32 user32)
    find_library(dwmapi dwmapi)
    find_library(gdi32 gdi32)
    set(arbalest_Link_Libraries
            ${arbalest_Link_Libraries}
            ${user32}
            ${dwmapi}
            ${gdi32})
ENDIF()

find_package(Qt5 COMPONENTS Widgets Gui OpenGL REQUIRED)
find_package(OpenGL REQUIRED)

# User Interface Compiling
qt5_wrap_ui(arbalest_PROCESSED_UIC resources/ui/MainWindow.ui)

# Meta-Object Compiling
file(GLOB arbalest_HEADERS_TO_MOC ./include/*)
qt5_wrap_cpp(arbalest_PROCESSED_MOCS  ${arbalest_HEADERS_TO_MOC} TARGET arbalest OPTIONS --no-notes)

# Resource Compiling
qt5_add_resources(arbalest_PROCESSED_RCC resources/resources.qrc)

include_directories(${arbalest_Include_Dirs})

add_executable(arbalest
        ${arbalest_Sources}
        ${arbalest_PROCESSED_UIC}
        ${arbalest_PROCESSED_MOCS}
        ${arbalest_PROCESSED_RCC})

target_link_libraries(arbalest ${arbalest_Link_Libraries})
