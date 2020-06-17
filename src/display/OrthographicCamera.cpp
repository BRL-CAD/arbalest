//
// Created by Sadeep on 09-Jun.
//

#include "OrthographicCamera.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>


#include<iostream>

using namespace std;

OrthographicCamera::OrthographicCamera() = default;

glm::mat4 OrthographicCamera::modelViewMatrix() const {
    auto rotationMatrixAroundX = glm::rotate(glm::radians(angleAroundAxes.x), axisX);
    auto rotationMatrixAroundY = glm::rotate(glm::radians(angleAroundAxes.y), axisY);
    auto rotationMatrixAroundZ = glm::rotate(glm::radians(angleAroundAxes.z), axisZ);
    auto rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;

    return glm::translate(rotationMatrix, eyePosition);
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

    float deltaAngleX = float(deltaX) / h;
    float deltaAngleY = float(deltaY) / h;
    if (thirdAxis) {
        angleAroundAxes.y += deltaAngleX * eyeRotationPerMouseDelta;
    } else {
        angleAroundAxes.z += deltaAngleX * eyeRotationPerMouseDelta;
    }
    angleAroundAxes.x += deltaAngleY * eyeRotationPerMouseDelta;

    //cout << angleAroundAxes.x << " " << angleAroundAxes.y << " " << angleAroundAxes.z << endl;
}

void OrthographicCamera::processMoveRequest(const int &deltaX, const int &deltaY) {
    if (deltaX < -mouseMaxDrag || deltaX > +mouseMaxDrag || deltaY < -mouseMaxDrag || deltaY > +mouseMaxDrag) {
        return;
    }

    auto rotationMatrixAroundZ = glm::rotate(glm::radians(-angleAroundAxes.z), axisZ);
    auto rotationMatrixAroundY = glm::rotate(glm::radians(-angleAroundAxes.y), axisY);
    auto rotationMatrixAroundX = glm::rotate(glm::radians(-angleAroundAxes.x), axisX);
    auto rotationMatrix = rotationMatrixAroundZ * rotationMatrixAroundY * rotationMatrixAroundX;

    glm::vec3 cameraRightDirection(rotationMatrix * glm::vec4(axisX, 1.0));
    eyePosition += float(deltaX) * eyeMovementPerMouseDelta * cameraRightDirection * zoom;

    glm::vec3 cameraUpDirection(rotationMatrix * glm::vec4(axisY, 1.0));
    eyePosition -= float(deltaY) * eyeMovementPerMouseDelta * cameraUpDirection * zoom;
}

void OrthographicCamera::processZoomRequest(const int &deltaWheelAngle) {
    float zoomFactor = 1 - zoomFactorMultiplier * float(deltaWheelAngle);
    zoom = pow(zoom, zoomFactor);
    if (zoom < zoomLowerBound) zoom = zoomLowerBound;
}
