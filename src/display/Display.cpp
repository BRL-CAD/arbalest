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
/** @file ArbDisplay.cpp */

#include "ArbDisplay.h"

#include <iostream>
#include <QScreen>
#include <QWidget>
#include <OrthographicCamera.h>
#include <include/Globals.h>
#include "ArbDisplayManager.h"
#include "GeometryRenderer.h"
#include "Utils.h"

using namespace std;


ArbDisplay::ArbDisplay(Document*  document)
    :document(document) {
    camera = new OrthographicCamera(document);
    displayManager = new ArbDisplayManager(*this);
    axesRenderer = new AxesRenderer();
    gridRenderer = new GridRenderer(this);

    bgColor = Globals::theme->getColor("$Color-GraphicsView");
    displayManager->setBGColor(bgColor.redF(),bgColor.greenF(),bgColor.blueF());

    makeCurrent();
    update();
}

ArbDisplay::~ArbDisplay() {
    delete camera;
    delete displayManager;
    delete axesRenderer;
}


void ArbDisplay::forceRerenderFrame() {
    makeCurrent();
    update();
}

int ArbDisplay::getW() const {
    return w;
}

int ArbDisplay::getH() const {
    return h;
}

const Document* ArbDisplay::getDocument() const
{
    return document;
}

ArbDisplayManager* ArbDisplay::getArbDisplayManager() const
{
	return displayManager;
}

void ArbDisplay::resizeGL(const int w, const int h) {

    double ratio = QGuiApplication::primaryScreen()->devicePixelRatio();

    camera->setWH(ratio*w,ratio*h);
    this->w = ratio*w;
    this->h = ratio*h;
}

void ArbDisplay::paintGL() {
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

void ArbDisplay::keyPressEvent( QKeyEvent *k ) {
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

OrthographicCamera *ArbDisplay::getCamera() const {
    return camera;
}
