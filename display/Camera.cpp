//
// Created by Sadeep on 09-Jun.
//

#include "Camera.h"
#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>


#include<iostream>
using namespace std;

Camera::Camera() = default;


glm::mat4x4 Camera::modelViewMatrix() const {
    glm::mat4 view;

    auto rotationMatrixAroundY = glm::rotate(glm::radians(angleAroundY), axisY);
    auto rotationMatrixAroundX = glm::rotate(glm::radians(angleAroundX), axisX);
    auto rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY;

    return glm::translate(rotationMatrix,eyePosition);
}

glm::mat4 Camera::projectionMatrix() const {
    glm::mat4 view = glm::perspective(glm::radians(fov),w/h,nearPlane,farPlane);
    return view;
}

void Camera::setWH(float w, float h) {
    this->w = w;
    this->h = h;
}

void Camera::processMouseDrag(int deltaX, int deltaY, bool rotate) {
    if (rotate) {
        float deltaAngleX = float(deltaX) * fov / h;
        float deltaAngleY = float(deltaY) * fov / h;
        angleAroundY += deltaAngleX;
        angleAroundX += deltaAngleY;
    }
    else{
        auto rotationMatrixAroundY = glm::rotate(glm::radians(-angleAroundY), axisY);
        auto rotationMatrixAroundX = glm::rotate(glm::radians(-angleAroundX), axisX);
        auto rotationMatrix = rotationMatrixAroundY * rotationMatrixAroundX;

        glm::vec3 cameraRightDirection(rotationMatrix * glm::vec4(axisX, 1.0));
        eyePosition += float(deltaX) * eyeMovementPerMouseMove * cameraRightDirection;

        glm::vec3 cameraUpDirection(rotationMatrix * glm::vec4(axisY, 1.0));
        eyePosition += float(-deltaY) * eyeMovementPerMouseMove * cameraUpDirection;
    }
}

void Camera::processMouseWheel(int deltaWheelAngle) {

    auto rotationMatrixAroundY = glm::rotate(glm::radians(-angleAroundY), axisY);
    auto rotationMatrixAroundX = glm::rotate(glm::radians(-angleAroundX), axisX);
    auto rotationMatrix = rotationMatrixAroundY * rotationMatrixAroundX;

    glm::vec3 cameraForwardDirection(rotationMatrix * glm::vec4(-axisZ, 1.0));

    eyePosition += -float(deltaWheelAngle) * eyeMovementPerWheelAngle * cameraForwardDirection;
}