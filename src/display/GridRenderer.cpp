
#include "GridRenderer.h"

#if defined(WIN32) && !defined(__CYGWIN__)
#  include<windows.h>
#endif

#if defined(__APPLE__)
#  include <OpenGL/gl.h>
#else
#  include <GL/gl.h>
#endif

#include <cmath>
#include <iostream>

#include "AxesRenderer.h"


void GridRenderer::render() {
    //todo implement line drawing in DisplayManager and use it instead of calling OpenGL here

    int lineCount = 1000;
    double lineGap;
    lineGap = .1 * std::pow(2.71,int(std::log(display->getCamera()->getVerticalSpan())));

    float length = lineCount*lineGap;
    float lineColor[] = {.3,.3,.3};

    display->getDisplayManager()->saveState();  

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    float alpha = sqrt(abs(std::fmod(abs(display->getCamera()->getAnglesAroundAxes()[0]),180)-90)/90.)*.7;
    if (abs(std::fmod(abs(display->getCamera()->getAnglesAroundAxes()[0]),180)-90)<4) {
        alpha = (abs(std::fmod(abs(display->getCamera()->getAnglesAroundAxes()[0]),180)-90)/90.)*.7;
    }

    glColor4f(lineColor[0],lineColor[1],lineColor[2],alpha);
    glBegin(GL_LINES);
    for (double i = -lineCount*lineGap/2; i < lineCount*lineGap/2; i+=lineGap){
        glVertex3f(i, length / 2, 0);
        glVertex3f(i, -length / 2, 0);
    }

    for (double i = -lineCount*lineGap/2; i < lineCount*lineGap/2; i+=lineGap){
        glVertex3f(length / 2,i, 0);
        glVertex3f(-length / 2,i, 0);
    }

    glEnd();

    display->getDisplayManager()->restoreState();
}



GridRenderer::GridRenderer(Display *display) : display(display) {}
