//
// Created by sadeep on 6/2/19.
//

#ifndef RT3_GRAPHICSVIEWOPENGL_H
#define RT3_GRAPHICSVIEWOPENGL_H


#include <QtWidgets/QOpenGLWidget>
#include <QtGui/QOpenGLFunctions>

#include <QWidget>

#include <QGLWidget>
#include <QtOpenGL>

#include <QMatrix4x4>

class GraphicsViewOpenGL: public QGLWidget{
public:

    GraphicsViewOpenGL();
    void  setVlist(struct bn_vlist *vp);
    void ogl_drawVList();

    bn_vlist * vp =0;

protected:
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();

    void keyPressEvent(QKeyEvent *k) override;


private:





};


#endif //RT3_GRAPHICSVIEWOPENGL_H
