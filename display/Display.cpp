//
// Created by Sadeep on 07-Jun.
//

#include "Display.h"
#include <glm/gtc/type_ptr.hpp>
#include <QCursor>
#include <GridRenderer.h>
#include "VectorListRenderer.h"
#define DEFAULT_LINE_WIDTH 0.1
#define RED 1.0, 0.0, 0.0
#define GREEN 0.0, 1.0, 0.0
#define BLUE 0.0, 0.0, 1.0
#define BG_COLOR .2,.2,.2,1
#define GRID_COLOR .5,.5,.5

using namespace std;

Display::Display() {
    camera = new Camera();
    vectorListRenderer = new VectorListRenderer();
    gridRenderer = new GridRenderer();
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

    glColor3f(1,.3,.3);
    for(auto vl: vectorLists){
        vectorListRenderer->render(vl,w,h);
    }

    gridRenderer->render();
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

std::vector<BRLCAD::VectorList *> &Display::getVectorLists() {
    return vectorLists;
}
