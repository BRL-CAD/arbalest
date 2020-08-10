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
#include "Utils.h"
#include <QDebug>

glm::mat4 OrthographicCamera::modelViewMatrix() const {
	const glm::mat4 rotationMatrixAroundX = glm::rotate(glm::radians(angleAroundAxes.x), axisX);
	const glm::mat4 rotationMatrixAroundY = glm::rotate(glm::radians(angleAroundAxes.y), axisY);
	const glm::mat4 rotationMatrixAroundZ = glm::rotate(glm::radians(angleAroundAxes.z), axisZ);
	const glm::mat4 rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;

    return glm::translate(rotationMatrix, -eyePosition);
}

glm::mat4 OrthographicCamera::projectionMatrix() const {
    return glm::ortho(-(verticalSpan/2) * w / h, (verticalSpan/2) * w / h, -verticalSpan/2, verticalSpan/2, nearPlane, farPlane);
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
    eyePosition -= static_cast<float>(deltaX) * eyeMovementPerMouseDelta * cameraRightDirection * verticalSpan;

    const glm::vec3 cameraUpDirection(rotationMatrix * glm::vec4(axisY, 1.0));
    eyePosition += static_cast<float>(deltaY) * eyeMovementPerMouseDelta * cameraUpDirection * verticalSpan;
}

void OrthographicCamera::processZoomRequest(const int &deltaWheelAngle) {
	const float zoomFactor = 1 - zoomFactorMultiplier * static_cast<float>(deltaWheelAngle);
    verticalSpan = pow(verticalSpan, zoomFactor);
    if (verticalSpan < zoomLowerBound) verticalSpan = zoomLowerBound;
}

glm::mat4 OrthographicCamera::modelViewMatrixNoTranslate() const {
	const glm::mat4 rotationMatrixAroundX = glm::rotate(glm::radians(angleAroundAxes.x), axisX);
	const glm::mat4 rotationMatrixAroundY = glm::rotate(glm::radians(angleAroundAxes.y), axisY);
	const glm::mat4 rotationMatrixAroundZ = glm::rotate(glm::radians(angleAroundAxes.z), axisZ);
    glm::mat4 rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;
    return rotationMatrix;
}


void OrthographicCamera::setEyePosition(float x, float y, float z)
{
    eyePosition.x = x;
    eyePosition.y = y;
    eyePosition.z = z;
}

glm::vec3 OrthographicCamera::getEyePosition()
{
    return eyePosition;
}

void OrthographicCamera::setAnglesAroundAxes(float x, float y, float z)
{
    angleAroundAxes.x = x;
    angleAroundAxes.y = y;
    angleAroundAxes.z = z;
}
glm::vec3 OrthographicCamera::getAnglesAroundAxes()
{
    return angleAroundAxes;
}

void OrthographicCamera::setZoom(const float zoom)
{
    this->verticalSpan = zoom;
}
double OrthographicCamera::getVerticalSpan()
{
    return this->verticalSpan;
}

void OrthographicCamera::centerToCurrentSelection() {
    BRLCAD::Vector3D a = document->getDatabase()->BoundingBoxMinima();
    BRLCAD::Vector3D b = document->getDatabase()->BoundingBoxMaxima();
    BRLCAD::Vector3D midPoint = (a+b) / 2;
    setEyePosition(midPoint.coordinates[0], midPoint.coordinates[1], midPoint.coordinates[2]);

    const BRLCAD::Vector3D volume = (a - b);
    double diagonalLength = vector3DLength(volume);
    if (diagonalLength > 0.001) setZoom(diagonalLength * 1.1);
    document->getDisplay()->forceRerenderFrame();
}

void OrthographicCamera::autoview() {
    document->getDatabase()->UnSelectAll();
    document->getObjectTree()->traverseSubTree(0, false, [this]
    (int objectId){
        switch(document->getObjectTree()->getObjectVisibility()[objectId]){
            case ObjectTree::Invisible:
                return false;
            case ObjectTree::SomeChildrenVisible:
                return true;
            case ObjectTree::FullyVisible:
                QString fullPath = document->getObjectTree()->getFullPathMap()[objectId];
                document->getDatabase()->Select(fullPath.toUtf8());
                return false;
        }
        return true;
    }
    );

    centerToCurrentSelection();
}

void OrthographicCamera::centerView(int objectId) {
    document->getDatabase()->UnSelectAll();
    QString fullPath = document->getObjectTree()->getFullPathMap()[objectId];
    document->getDatabase()->Select(fullPath.toUtf8());
    centerToCurrentSelection();
}

