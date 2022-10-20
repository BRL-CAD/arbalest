/*                       D I S P L A Y . H
 * BRL-CAD
 *
 * Copyright (c) 2020 United States Government as represented by
 * the U.S. Army Research Laboratory.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * version 2.1 as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this file; see the file named COPYING for more
 * information.
 */
/** @file Display.h */


#ifndef RT3_DISPLAY_H
#define RT3_DISPLAY_H

#include <include/Globals.h>

#include <QtWidgets/QOpenGLWidget>
#include "AxesRenderer.h"
#include <QMouseEvent>
#include <include/GridRenderer.h>
#include "Document.h"
#include "DisplayManager.h"
#include "OrthographicCamera.h"
#include "Globals.h"
#include "QSSPreprocessor.h"

class Document;
class DisplayManager;
class GeometryRenderer;
class OrthographicCamera;
class GridRenderer;


class Display : public QOpenGLWidget{

public:
    Display(Document * document);
    virtual ~Display();

    void forceRerenderFrame();

    int getW() const;
    int getH() const;
    const Document* getDocument() const;
    OrthographicCamera* getCamera() const;
    DisplayManager* getDisplayManager() const;

    bool gridEnabled = false;

protected:
    void resizeGL(int w, int h) override;
    void paintGL() override;

    void keyPressEvent(QKeyEvent *k) override ;

private:
    Document * document;
    int w = 400;
    int h = 400;
    float keyPressSimulatedMouseMoveDistance = 8;
    QColor bgColor;

    OrthographicCamera  *camera;
    DisplayManager *displayManager;
    AxesRenderer * axesRenderer;
    GridRenderer * gridRenderer;
};


#endif //RT3_DISPLAY_H
