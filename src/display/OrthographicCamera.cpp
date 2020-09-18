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
#include "Utils.h"
#include <cmath>
#include <QDebug>

QMatrix4x4 OrthographicCamera::modelViewMatrix() const {
    QMatrix4x4 rotationMatrixAroundX;
    rotationMatrixAroundX.rotate(angleAroundAxes.x(), axisX);
    QMatrix4x4 rotationMatrixAroundY;
    rotationMatrixAroundY.rotate(angleAroundAxes.y(), axisY);
    QMatrix4x4 rotationMatrixAroundZ;
    rotationMatrixAroundZ.rotate(angleAroundAxes.z(), axisZ);
    QMatrix4x4 rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;
    rotationMatrix.translate(-eyePosition);

    return rotationMatrix;
}

QMatrix4x4 OrthographicCamera::projectionMatrix() const {
    QMatrix4x4 ret;
    ret.ortho(-(verticalSpan/2) * w / h, (verticalSpan/2) * w / h, -verticalSpan/2, verticalSpan/2, nearPlane, farPlane);

    return ret;
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
        angleAroundAxes.setY(angleAroundAxes.y() + deltaAngleX * eyeRotationPerMouseDelta);
    } else {
        angleAroundAxes.setZ(angleAroundAxes.z() + deltaAngleX * eyeRotationPerMouseDelta);
    }
    angleAroundAxes.setX(angleAroundAxes.x() + deltaAngleY * eyeRotationPerMouseDelta);

}

void OrthographicCamera::processMoveRequest(const int &deltaX, const int &deltaY) {
    if (deltaX < -mouseMaxDrag || deltaX > +mouseMaxDrag || deltaY < -mouseMaxDrag || deltaY > +mouseMaxDrag) {
        return;
    }

    QMatrix4x4 rotationMatrixAroundZ;
    rotationMatrixAroundZ.rotate(-angleAroundAxes.z(), axisZ);
    QMatrix4x4 rotationMatrixAroundY;
    rotationMatrixAroundY.rotate(-angleAroundAxes.y(), axisY);
    QMatrix4x4 rotationMatrixAroundX;
    rotationMatrixAroundX.rotate(-angleAroundAxes.x(), axisX);
    QMatrix4x4 rotationMatrix = rotationMatrixAroundZ * rotationMatrixAroundY * rotationMatrixAroundX;

    QVector3D cameraRightDirection(rotationMatrix * axisX);
    eyePosition -= static_cast<float>(deltaX) * eyeMovementPerMouseDelta * cameraRightDirection * verticalSpan;

    QVector3D cameraUpDirection(rotationMatrix * axisY);
    eyePosition += static_cast<float>(deltaY) * eyeMovementPerMouseDelta * cameraUpDirection * verticalSpan;
}

void OrthographicCamera::processZoomRequest(const int &deltaWheelAngle) {
	const float zoomFactor = 1 - zoomFactorMultiplier * static_cast<float>(deltaWheelAngle);
    verticalSpan = pow(verticalSpan, zoomFactor);
    if (verticalSpan < zoomLowerBound) verticalSpan = zoomLowerBound;
    if (verticalSpan > zoomUpperBound) verticalSpan = zoomUpperBound;
}

QMatrix4x4 OrthographicCamera::modelViewMatrixNoTranslate() const {
    QMatrix4x4 rotationMatrixAroundX;
    rotationMatrixAroundX.rotate(angleAroundAxes.x(), axisX);
    QMatrix4x4 rotationMatrixAroundY;
    rotationMatrixAroundY.rotate(angleAroundAxes.y(), axisY);
    QMatrix4x4 rotationMatrixAroundZ;
    rotationMatrixAroundZ.rotate(angleAroundAxes.z(), axisZ);
    QMatrix4x4 rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;
    return rotationMatrix;
}


void OrthographicCamera::setEyePosition(float x, float y, float z)
{
    eyePosition.setX(x);
    eyePosition.setY(y);
    eyePosition.setZ(z);
}

QVector3D OrthographicCamera::getEyePosition()
{
    return eyePosition;
}

void OrthographicCamera::setAnglesAroundAxes(float x, float y, float z)
{
    angleAroundAxes.setX(x);
    angleAroundAxes.setY(y);
    angleAroundAxes.setZ(z);
}
QVector3D OrthographicCamera::getAnglesAroundAxes()
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

