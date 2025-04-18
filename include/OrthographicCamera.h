/*            O R T H O G R A P H I C C A M E R A . H
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
/** @file OrthographicCamera.h */


#ifndef RT3_OrthographicCAMERA_H
#define RT3_OrthographicCAMERA_H


#include "Camera.h"
#include "Document.h"

class Document;
class OrthographicCamera : Camera{

private:
    Document * document;
    const QVector3D initialEyePosition = QVector3D(0.0f, 0.0f, 0.0f);
    const QVector3D initialAngleAroundAxes = QVector3D(295.0f, 0.0f, 235.0f);

    const float nearPlane = -2000000.0f;
    const float farPlane = 2000000.0f;
    const float eyeMovementPerMouseDelta = 0.002075f;
    const float eyeRotationPerMouseDelta = 120.f;
    const float zoomFactorMultiplier = 0.001;
    const float zoomLowerBound = 0.00001;
    const float zoomUpperBound = 1000000000000000;
    const int mouseMaxDrag = 500;

    QVector3D eyePosition = initialEyePosition; // Camera coordinates
    float verticalSpan = 600;

    QVector3D angleAroundAxes = initialAngleAroundAxes; // Camera direction in degrees
    float w = 400, h = 400;         // Viewport width and height.

public:
    explicit OrthographicCamera(Document * document) : document(document){}
    virtual ~OrthographicCamera() = default;

    void setWH(float w, float h);

    void processMoveRequest(const int &deltaX, const int &deltaY) ;

    void processRotateRequest(const int &deltaX, const int &deltaY, const bool &thirdAxis) ;

    void processZoomRequest(const int &deltaWheelAngle) ;

    QMatrix4x4 modelViewMatrix() const ;

    QMatrix4x4 modelViewMatrixNoTranslate() const ;

    QMatrix4x4 projectionMatrix() const ;


    void setEyePosition(float x, float y, float z) ;
    void setAnglesAroundAxes(float x, float y, float z) ;
    QVector3D getAnglesAroundAxes();

    void setZoom(float zoom);

    void autoview();

    void centerView(int objectId);

    void centerToCurrentSelection();

    QVector3D getEyePosition();

    double getVerticalSpan();
};


#endif //RT3_OrthographicCAMERA_H
