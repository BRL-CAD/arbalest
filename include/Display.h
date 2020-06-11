//
// Created by Sadeep on 07-Jun.
//

#ifndef RT3_DISPLAY_H
#define RT3_DISPLAY_H

#include <QtWidgets/QOpenGLWidget>
#include "Camera.h"
#include <QMouseEvent>

class Display : public QOpenGLWidget{

public:
    Display();
    Camera *camera;
    void refresh();

protected:
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void mousePressEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void keyPressEvent(QKeyEvent *k) override ;

private:
    int prevMouseX = -1,  prevMouseY = -1;
    bool skipNextMouseMoveEvent = false;
    float keyPressSimulatedMouseMoveDistance = 8;

};


#endif //RT3_DISPLAY_H
