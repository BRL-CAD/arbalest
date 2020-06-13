//
// Created by Sadeep on 07-Jun.
//

#include "Display.h"
#include <glm/gtc/type_ptr.hpp>
#include <QCursor>
#include "VectorListRenderer.h"
#define DEFAULT_LINE_WIDTH 0.1
#define RED 1.0, 0.0, 0.0
#define GREEN 0.0, 1.0, 0.0
#define BLUE 0.0, 0.0, 1.0
#define BG_COLOR .2,.2,.2,1
#define GRID_COLOR .5,.5,.5

#include<iostream>
using namespace std;

Display::Display() {
    camera = new Camera();
};

void Display::resizeGL(int w, int h) {
    camera->setWH(w,h);
    this->w = w;
    this->h = h;
}

void Display::paintGL() {
    glMatrixMode(GL_MODELVIEW);
    glLoadMatrixf((const float*)glm::value_ptr(camera->modelViewMatrix()));
    glMatrixMode(GL_PROJECTION);
    glLoadMatrixf((const float*)glm::value_ptr(camera->projectionMatrix()));

    glClearColor(BG_COLOR);
    glClear(GL_COLOR_BUFFER_BIT);

    glLineWidth(DEFAULT_LINE_WIDTH);
    glColor3f(GRID_COLOR);
    //glEnable(GL_LINE_SMOOTH);

    float M = 1000;

    for (int c=-1000;c<1000;c++){

        glBegin(GL_LINES);
        glVertex3f(c, 0, -M);
        glVertex3f(c, 0, M);
        glEnd();
    }
    for (int c=-1000;c<1000;c++){

        glBegin(GL_LINES);
        glVertex3f(-M, 0, c);
        glVertex3f(M, 0, c);
        glEnd();
    }

    glBegin(GL_LINES);
    glVertex3f(0, 1000, 0);
    glVertex3f(0, -1000, 0);
    glEnd();

//
//
//    glBegin(GL_QUADS); // of the color cube
//
//    // Top-face
//    glColor3f(0.0f, 1.0f, 0.0f); // green
//    glVertex3f(1.0f, 1.0f, -1.0f);
//    glVertex3f(-1.0f, 1.0f, -1.0f);
//    glVertex3f(-1.0f, 1.0f, 1.0f);
//    glVertex3f(1.0f, 1.0f, 1.0f);
//
//    // Bottom-face
//    glColor3f(1.0f, 0.5f, 0.0f); // orange
//    glVertex3f(1.0f, -1.0f, 1.0f);
//    glVertex3f(-1.0f, -1.0f, 1.0f);
//    glVertex3f(-1.0f, -1.0f, -1.0f);
//    glVertex3f(1.0f, -1.0f, -1.0f);
//
//    // Front-face
//    glColor3f(1.0f, 0.0f, 0.0f); // red
//    glVertex3f(1.0f, 1.0f, 1.0f);
//    glVertex3f(-1.0f, 1.0f, 1.0f);
//    glVertex3f(-1.0f, -1.0f, 1.0f);
//    glVertex3f(1.0f, -1.0f, 1.0f);
//
//    // Back-face
//    glColor3f(1.0f, 1.0f, 0.0f); // yellow
//    glVertex3f(1.0f, -1.0f, -1.0f);
//    glVertex3f(-1.0f, -1.0f, -1.0f);
//    glVertex3f(-1.0f, 1.0f, -1.0f);
//    glVertex3f(1.0f, 1.0f, -1.0f);
//
//    // Left-face
//    glColor3f(0.0f, 0.0f, 1.0f); // blue
//    glVertex3f(-1.0f, 1.0f, 1.0f);
//    glVertex3f(-1.0f, 1.0f, -1.0f);
//    glVertex3f(-1.0f, -1.0f, -1.0f);
//    glVertex3f(-1.0f, -1.0f, 1.0f);
//
//    // Right-face
//    glColor3f(1.0f, 0.0f, 1.0f); // magenta
//    glVertex3f(1.0f, 1.0f, -1.0f);
//    glVertex3f(1.0f, 1.0f, 1.0f);
//    glVertex3f(1.0f, -1.0f, 1.0f);
//    glVertex3f(1.0f, -1.0f, -1.0f);
//
//    glEnd(); // of the color cube

    for(auto vl: vectorLists){
        VectorListRenderer::render(vl,this);
    }
}

void Display::refresh() {
    makeCurrent();
    update();
}


void Display::mouseMoveEvent(QMouseEvent *event) {
    int x = event->x();
    int y = event->y();
    int globalX = event->globalX();
    int globalY = event->globalY();

    bool resetX = false, resetY = false;

    if(prevMouseX != -1 && prevMouseY != -1 && (event->buttons() & (Qt::MiddleButton|Qt::RightButton))) {
        if (skipNextMouseMoveEvent){
            skipNextMouseMoveEvent = false;
            return;
        }

        if(event->buttons() & (Qt::MiddleButton)){
            camera->processMouseDrag(x- prevMouseX, y - prevMouseY, true);
        }
        else{
            camera->processMouseDrag(x- prevMouseX, y - prevMouseY, false);
        }
        refresh();

        auto topLeft = mapToGlobal(QPoint(0,0));
        auto bottomRight = mapToGlobal(QPoint(size().width(),size().height()));

        int newX = -1;
        int newY = -1;

        if (globalX <= topLeft.x()) {
            newX = bottomRight.x()-1;
            resetX = true;
        }
        if (globalX >= bottomRight.x()) {
            newX = topLeft.x()+1;
            resetX = true;
        }
        if (globalY <= topLeft.y()) {
            newY = bottomRight.y()-1;
            resetY = true;
        }
        if (globalY >= bottomRight.y()) {
            newY = topLeft.y()+1;
            resetY = true;
        }

        if (resetX || resetY) {
            prevMouseX = resetX ? mapFromGlobal(QPoint(newX,newY)).x() : x;
            prevMouseY = resetY ? mapFromGlobal(QPoint(newX,newY)).y() : y;
            QCursor::setPos(resetX ? newX : globalX, resetY ? newY : globalY);
            skipNextMouseMoveEvent = true;
        }
    }

    if(!resetX && !resetY) {
        prevMouseX = x;
        prevMouseY = y;
    }
}

void Display::mousePressEvent(QMouseEvent *event) {
    prevMouseX = event->x();
    prevMouseY = event->y();
}

void Display::mouseReleaseEvent(QMouseEvent *event) {
    prevMouseX = -1;
    prevMouseY = -1;
}

void Display::wheelEvent(QWheelEvent *event) {
    if (event->phase() == Qt::NoScrollPhase || event->phase() == Qt::ScrollUpdate || event->phase() == Qt::ScrollMomentum) {
        camera->processMouseWheel(event->angleDelta().y() / 8);
        refresh();
    }
}

void Display::keyPressEvent( QKeyEvent *k ) {
    switch (k->key()) {
        case Qt::Key_Up:
            camera->processMouseDrag(0, keyPressSimulatedMouseMoveDistance, false);
            refresh();
            break;
        case Qt::Key_Down:
            camera->processMouseDrag(0, -keyPressSimulatedMouseMoveDistance, false);
            refresh();
            break;
        case Qt::Key_Left:
            camera->processMouseDrag(keyPressSimulatedMouseMoveDistance, 0, false);
            refresh();
            break;
        case Qt::Key_Right:
            camera->processMouseDrag(-keyPressSimulatedMouseMoveDistance, 0, false);
            refresh();
            break;
    }
}

int Display::getW() const {
    return w;
}

int Display::getH() const {
    return h;
}

std::vector<BRLCAD::VectorList *> &Display::getVectorLists() {
    return vectorLists;
}
