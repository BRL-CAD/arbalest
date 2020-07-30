/*          O R T H O G R A P H I C C A M E R A . C P P
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
/** @file OrthographicCamera.cpp */

#include "OrthographicCamera.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

OrthographicCamera::OrthographicCamera() = default;

glm::mat4 OrthographicCamera::modelViewMatrix() const {
	const glm::mat4 rotationMatrixAroundX = glm::rotate(glm::radians(angleAroundAxes.x), axisX);
	const glm::mat4 rotationMatrixAroundY = glm::rotate(glm::radians(angleAroundAxes.y), axisY);
	const glm::mat4 rotationMatrixAroundZ = glm::rotate(glm::radians(angleAroundAxes.z), axisZ);
	const glm::mat4 rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;

    return glm::translate(rotationMatrix, -eyePosition);
}

glm::mat4 OrthographicCamera::projectionMatrix() const {
    return glm::ortho(-zoom * w / h, zoom * w / h, -zoom, zoom, nearPlane, farPlane);
}

void OrthographicCamera::setWH(float w, float h) {
    this->w = w;
    this->h = h;
}

void OrthographicCamera::processRotateRequest(const int &deltaX, const int &deltaY, const bool &thirdAxis) {
    if (deltaX < -mouseMaxDrag || deltaX > +mouseMaxDrag || deltaY < -mouseMaxDrag || deltaY > +mouseMaxDrag) {
        return;
    }

    const float deltaAngleX = float(deltaX) / h;
    const float deltaAngleY = float(deltaY) / h;
    if (thirdAxis) {
        angleAroundAxes.y += deltaAngleX * eyeRotationPerMouseDelta;
    } else {
        angleAroundAxes.z += deltaAngleX * eyeRotationPerMouseDelta;
    }
    angleAroundAxes.x += deltaAngleY * eyeRotationPerMouseDelta;

}

void OrthographicCamera::processMoveRequest(const int &deltaX, const int &deltaY) {
    if (deltaX < -mouseMaxDrag || deltaX > +mouseMaxDrag || deltaY < -mouseMaxDrag || deltaY > +mouseMaxDrag) {
        return;
    }

    const glm::mat4 rotationMatrixAroundZ = glm::rotate(glm::radians(-angleAroundAxes.z), axisZ);
    const glm::mat4 rotationMatrixAroundY = glm::rotate(glm::radians(-angleAroundAxes.y), axisY);
    const glm::mat4 rotationMatrixAroundX = glm::rotate(glm::radians(-angleAroundAxes.x), axisX);
    const glm::mat4 rotationMatrix = rotationMatrixAroundZ * rotationMatrixAroundY * rotationMatrixAroundX;

    const glm::vec3 cameraRightDirection(rotationMatrix * glm::vec4(axisX, 1.0));
    eyePosition -= static_cast<float>(deltaX) * eyeMovementPerMouseDelta * cameraRightDirection * zoom;

    const glm::vec3 cameraUpDirection(rotationMatrix * glm::vec4(axisY, 1.0));
    eyePosition += static_cast<float>(deltaY) * eyeMovementPerMouseDelta * cameraUpDirection * zoom;
}

void OrthographicCamera::processZoomRequest(const int &deltaWheelAngle) {
	const float zoomFactor = 1 - zoomFactorMultiplier * static_cast<float>(deltaWheelAngle);
    zoom = pow(zoom, zoomFactor);
    if (zoom < zoomLowerBound) zoom = zoomLowerBound;
}

glm::mat4 OrthographicCamera::modelViewMatrixNoTranslate() const {
	const glm::mat4 rotationMatrixAroundX = glm::rotate(glm::radians(angleAroundAxes.x), axisX);
	const glm::mat4 rotationMatrixAroundY = glm::rotate(glm::radians(angleAroundAxes.y), axisY);
	const glm::mat4 rotationMatrixAroundZ = glm::rotate(glm::radians(angleAroundAxes.z), axisZ);
    glm::mat4 rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;
    return rotationMatrix;
}

glm::mat4 OrthographicCamera::projectionMatrix(float x, float y) const {
    return glm::ortho(-x, x, -y, y, nearPlane, farPlane);
}

void OrthographicCamera::setEyePosition(float x, float y, float z)
{
    eyePosition.x = x;
    eyePosition.y = y;
    eyePosition.z = z;
}

