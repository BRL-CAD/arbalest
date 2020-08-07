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


#include <glm/detail/type_mat4x4.hpp>
#include <brlcad/cicommon.h>
#include "Camera.h"
#include "Document.h"

class Document;
class OrthographicCamera : Camera{
private:
    Document * document;
    const glm::vec3 initialEyePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    const glm::vec3 initialAngleAroundAxes = glm::vec3(295.0f, 0.0f, 235.0f);

    const float nearPlane = -2000000.0f;
    const float farPlane = 2000000.0f;
    const float eyeMovementPerMouseDelta = 0.002075f;
    const float eyeRotationPerMouseDelta = 120.f;
    const float zoomFactorMultiplier = 0.001;
    const float zoomLowerBound = 0.00001;
    const int mouseMaxDrag = 500;

    glm::vec3 eyePosition = initialEyePosition; // Camera coordinates
    float verticalSpan = 600;

    glm::vec3 angleAroundAxes = initialAngleAroundAxes; // Camera direction in degrees
    float w = 400, h = 400;         // Display width and height.

public:
    explicit OrthographicCamera(Document * document) : document(document){}
    virtual ~OrthographicCamera() = default;

    void setWH(float w, float h);

    void processMoveRequest(const int &deltaX, const int &deltaY) ;

    void processRotateRequest(const int &deltaX, const int &deltaY, const bool &thirdAxis) ;

    void processZoomRequest(const int &deltaWheelAngle) ;

    glm::mat4 modelViewMatrix() const ;

    glm::mat4 modelViewMatrixNoTranslate() const ;

    glm::mat4 projectionMatrix() const ;

    glm::mat4 projectionMatrix(float x, float y) const ;

    void setEyePosition(float x, float y, float z) ;

    void setZoom(float zoom);

    void autoview();

    void centerView(int objectId);

    void centerToCurrentSelection();
};


#endif //RT3_OrthographicCAMERA_H
