/*                     D I S P L A Y . C P P
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
/** @file Viewport.cpp */

#include "Viewport.h"

#include <iostream>
#include <QScreen>
#include <QWidget>
#include <OrthographicCamera.h>
#include <include/Globals.h>
#include "ViewportManager.h"
#include "GeometryRenderer.h"
#include "Utils.h"

using namespace std;


Viewport::Viewport(Document*  document)
    :document(document) {
    camera = new OrthographicCamera(document);
    displayManager = new ViewportManager(*this);
    axesRenderer = new AxesRenderer();
    gridRenderer = new GridRenderer(this);

    bgColor = Globals::theme->getColor("$Color-Viewport");
    displayManager->setBGColor(bgColor.redF(),bgColor.greenF(),bgColor.blueF());

    makeCurrent();
    update();
}

Viewport::~Viewport() {
    delete camera;
    delete displayManager;
    delete axesRenderer;
}


void Viewport::forceRerenderFrame() {
    makeCurrent();
    update();
}

int Viewport::getW() const {
    return w;
}

int Viewport::getH() const {
    return h;
}

const Document* Viewport::getDocument() const
{
    return document;
}

ViewportManager* Viewport::getViewportManager() const
{
	return displayManager;
}

void Viewport::resizeGL(const int w, const int h) {

    double ratio = QGuiApplication::primaryScreen()->devicePixelRatio();

    camera->setWH(ratio*w,ratio*h);
    this->w = ratio*w;
    this->h = ratio*h;
}

void Viewport::paintGL() {
    displayManager->drawBegin();

    glViewport(0,0,w,h);
    displayManager->loadMatrix(camera->modelViewMatrix().data());
    displayManager->loadPMatrix(camera->projectionMatrix().data());
    document->getGeometryRenderer()->render();
    if(gridEnabled)gridRenderer->render();

    glViewport(w*.88,h*.02,w/10,w/10);
    displayManager->loadMatrix(camera->modelViewMatrixNoTranslate().data());
    QMatrix4x4 orthoMtx;
    orthoMtx.ortho(-100.f, 100.f, -100.0f, 100.0f, -1000.f,1000.f);
    displayManager->loadPMatrix(orthoMtx.data());
    axesRenderer->render();
}

void Viewport::keyPressEvent( QKeyEvent *k ) {
    switch (k->key()) {
        case Qt::Key_Up:
            camera->processMoveRequest(0, keyPressSimulatedMouseMoveDistance);
            forceRerenderFrame();
            break;
        case Qt::Key_Down:
            camera->processMoveRequest(0, -keyPressSimulatedMouseMoveDistance);
            forceRerenderFrame();
            break;
        case Qt::Key_Left:
            camera->processMoveRequest(keyPressSimulatedMouseMoveDistance, 0);
            forceRerenderFrame();
            break;
        case Qt::Key_Right:
            camera->processMoveRequest(-keyPressSimulatedMouseMoveDistance, 0);
            forceRerenderFrame();
            break;
    }
}

OrthographicCamera *Viewport::getCamera() const {
    return camera;
}
