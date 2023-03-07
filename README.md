# Arbalest

## What the project is
The project aims to create a geometry editor for BRL-CAD. The project should ideally be an improvement over the existing editors MGED and Archer.

## Building
1. Install and configure Qt. Qt5.14.2 has been used for development of this project.
2. Clone BRL-CAD (`git clone https://github.com/BRL-CAD/brlcad.git`), build, and install it
3. Clone MOOSE (`git clone https://github.com/BRL-CAD/MOOSE.git`), build, and install it
4. Clone this project (`git clone https://github.com/BRL-CAD/arbalest.git`)
5. When running the CMake configuration, add BRLCAD_MOOSE_DIR to it and set this entry to the folder of your MOOSE installation
6. Build and run the target arbalest.

## What has been implemented and how to use it
You can keep multiple .g files open. Opened files will be displayed as tabs.

Top objects are displayed in the right pane.

Rotate camera by dragging with mouse middle pressed.

Move camera by dragging with mouse right pressed.

Use mouse wheel to go forward backward.
