//
// Created by Sadeep on 07-Jun.
//

#ifndef RT3_DISPLAY_H
#define RT3_DISPLAY_H

#include <QtWidgets/QOpenGLWidget>
#include "Camera.h"
#include "VectorListRenderer.h"
#include "GridRenderer.h"
#include <QMouseEvent>
#include <brlcad/VectorList.h>

class Display : public QOpenGLWidget{

public:
    Display();
    Camera *camera;
    void refresh();
    std::vector<BRLCAD::VectorList *> &getVectorLists();

protected:
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *k) override ;


private:
    int w = 400, h =400;
    int prevMouseX = -1,  prevMouseY = -1;
    bool skipNextMouseMoveEvent = false;
    float keyPressSimulatedMouseMoveDistance = 8;
    std::vector<BRLCAD::VectorList *> vectorLists;
    VectorListRenderer *vectorListRenderer;
    GridRenderer *gridRenderer;
};


#endif //RT3_DISPLAY_H
