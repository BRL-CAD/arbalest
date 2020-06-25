//
// Created by Sadeep on 07-Jun.
//

#ifndef RT3_DISPLAY_H
#define RT3_DISPLAY_H

#include <QtWidgets/QOpenGLWidget>
#include "Camera.h"
#include "GridRenderer.h"
#include <QMouseEvent>
#include <brlcad/cicommon.h>
#include <brlcad/VectorList.h>
#include <brlcad/MemoryDatabase.h>

class GeometryRenderer;


class Display : public QOpenGLWidget{

public:
    explicit Display(const BRLCAD::MemoryDatabase *database);

    Camera *camera;
    void refreshGeometry();
    void refresh();

    int getW() const;
    int getH() const;

    const BRLCAD::MemoryDatabase *getDatabase() const;

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
    GeometryRenderer *geometryRenderer;
    GridRenderer *gridRenderer;
    const BRLCAD::MemoryDatabase *database;
};


#endif //RT3_DISPLAY_H
