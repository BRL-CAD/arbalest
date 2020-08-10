#ifndef GLOBALS_H
#define GLOBALS_H

#include "QSSPreprocessor.h"
#include "MainWindow.h"

class MainWindow;

class Globals{
public:
    static QSSPreprocessor *theme;
    static MainWindow *mainWindow;
};


#endif