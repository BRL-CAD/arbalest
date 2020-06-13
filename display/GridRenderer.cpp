//
// Created by Sadeep on 13-Jun.
//

#ifdef _WIN32
#include <windows.h>
#endif

#include "GridRenderer.h"
#include <gl/GL.h>
#include <brlcad/bn/vlist.h>

#define DEFAULT_LINE_WIDTH 0.1
#define RED 1.0, 0.0, 0.0
#define GREEN 0.0, 1.0, 0.0
#define BLUE 0.0, 0.0, 1.0
#define BG_COLOR .2,.2,.2,1
#define GRID_COLOR .5,.5,.5

void GridRenderer::render() {

    glLineWidth(DEFAULT_LINE_WIDTH);
    glColor3f(GRID_COLOR);
//glEnable(GL_LINE_SMOOTH);

    float M = 1000;

    for (int c = -1000; c < 1000; c++) {

        glBegin(GL_LINES);
        glVertex3f(c, 0, -M);
        glVertex3f(c, 0, M);
        glEnd();
    }
    for (int c = -1000; c < 1000; c++) {

        glBegin(GL_LINES);
        glVertex3f(-M, 0, c);
        glVertex3f(M, 0, c);
        glEnd();
    }

    glBegin(GL_LINES);
    glVertex3f(0, 1000, 0);
    glVertex3f(0, -1000, 0);
    glEnd();
}