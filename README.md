# Arbalest

## What the project is
The project aims to create a geometry editor for BRL-CAD. The project should ideally be an improvement over the existing editors MGED and Archer.

## Building
1. Install and configure Qt. Qt5.14.2 has been used for development of this project.
2. Clone BRL-CAD (`git clone https://github.com/BRL-CAD/brlcad.git`), build, and install it
3. Clone rt-cubed (`git clone https://github.com/BRL-CAD/rt-cubed.git`) and build by following its README. If you have done correctly you should be able to run the target QtGUI in rt-cubed (just for verification).
4. Go to the src directory of rt-cubed repository in terminal.
5. Clone this project into src. (`git clone https://github.com/BRL-CAD/arbalest.git`)
6. Add this project to rt-cubed's CMakeList file. (append "ADD_SUBDIRECTORY(./arbalest)" to the end of rt-cubed\src\CMakeLists.txt)
7. Build and run the target arbalest.

## What has been implemented and how to use it
You can keep multiple .g files open. Opened files will be displayed as tabs.

Top objects are displayed in the right pane.

Rotate camera by dragging with mouse middle pressed.

Move camera by dragging with mouse right pressed.

Use mouse wheel to go forward backward.
