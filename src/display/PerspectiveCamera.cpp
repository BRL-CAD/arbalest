//
// Created by Sadeep on 09-Jun.
//

#include "PerspectiveCamera.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>

#include<iostream>
using namespace std;

PerspectiveCamera::PerspectiveCamera() = default;

glm::mat4 PerspectiveCamera::modelViewMatrix() const {
    auto rotationMatrixAroundX = glm::rotate(glm::radians(angleAroundAxes.x), axisX);
    auto rotationMatrixAroundY = glm::rotate(glm::radians(angleAroundAxes.y), axisY);
    auto rotationMatrixAroundZ = glm::rotate(glm::radians(angleAroundAxes.z), axisZ);
    return glm::translate(rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ, eyePosition);
}

glm::mat4 PerspectiveCamera::projectionMatrix() const {
    return glm::perspective(glm::radians(fov),w/h,nearPlane,farPlane);
}

void PerspectiveCamera::setWH(float w, float h) {
    this->w = w;
    this->h = h;
}

void PerspectiveCamera::processRotateRequest(const int &deltaX, const int &deltaY, const bool &thirdAxis) {
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

    cout << angleAroundAxes.x << " " << angleAroundAxes.y << " " << angleAroundAxes.z << endl;
}

void PerspectiveCamera::processMoveRequest(const int & deltaX, const int & deltaY){
    if (deltaX < -mouseMaxDrag || deltaX > +mouseMaxDrag || deltaY < -mouseMaxDrag || deltaY > +mouseMaxDrag){
        return;
    }

    auto rotationMatrixAroundY = glm::rotate(glm::radians(-angleAroundAxes.y), axisY);
    auto rotationMatrixAroundX = glm::rotate(glm::radians(-angleAroundAxes.x), axisX);
    auto rotationMatrix = rotationMatrixAroundY * rotationMatrixAroundX;

    glm::vec3 PerspectiveCameraRightDirection(rotationMatrix * glm::vec4(axisX, 1.0));
    eyePosition += float(deltaX) * eyeMovementPerMouseDelta * PerspectiveCameraRightDirection;

    glm::vec3 PerspectiveCameraUpDirection(rotationMatrix * glm::vec4(axisY, 1.0));
    eyePosition += float(-deltaY) * eyeMovementPerMouseDelta * PerspectiveCameraUpDirection;
}

void PerspectiveCamera::processZoomRequest(const int & deltaWheelAngle) {
    auto rotationMatrixAroundY = glm::rotate(glm::radians(-angleAroundAxes.y), axisY);
    auto rotationMatrixAroundX = glm::rotate(glm::radians(-angleAroundAxes.x), axisX);
    auto rotationMatrix = rotationMatrixAroundY * rotationMatrixAroundX;

    glm::vec3 PerspectiveCameraForwardDirection(rotationMatrix * glm::vec4(-axisZ, 1.0));

    eyePosition += -float(deltaWheelAngle) * eyeMovementPerWheelAngle * PerspectiveCameraForwardDirection;
}
