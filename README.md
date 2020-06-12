# Arbalest

## What the project is
The project aims to create a geometry editor for BRL-CAD. The project should ideally be an improvement over the existing editors MGED and Archer.

## Building
1. Install and configure Qt. Qt5.14.2 has been used for development of this project.
2. Clone BRL-CAD (https://svn.code.sf.net/p/brlcad/code/brlcad/trunk brlcad-code) and build
3. Clone RT^3 (https://sourceforge.net/p/brlcad/code/HEAD/tree/rt%5E3/) and build by following its README. If you have done correctly you should be able to run the target QtGUI in RT^3 (just for verification).
4. Go to \rt3\src of RT^3 repository in terminal. (cd \rt3\src)
5. Clone this project into \rt3\src. (git clone https://github.com/sadeepdarshana/arbalest)
6. Add this project to rt3's CMakeList file. (append "ADD_SUBDIRECTORY(./arbalest)" to the end of \rt3\src\CMakeLists.txt)
7. Build and run the target arbalest.

## What has been implemented and how to use it
You can keep multiple .g files open. Opened files will be displayed as tabs.

Top objects are displayed in the right pane. (needs to be replaced by a tree viewer so I can display all)

Grid looks ugly. Needs to make it gradually dissapear the further it is from camera.

Rotate camera by dragging with mouse middle pressed.

Move camera by dragging with mouse right pressed.

Use mouse wheel to go forward backward.

Displaying the model and objects list aren't working since I'm in the middle of some stuff. They should be working if you try an earlier commit.
