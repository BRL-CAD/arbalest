//
// Created by Sadeep on 09-Jun.
//

#ifndef RT3_PerspectiveCamera_H
#define RT3_PerspectiveCamera_H


#include <glm/detail/type_mat4x4.hpp>
#include "Camera.h"


class PerspectiveCamera : public Camera {
private:
    const glm::vec3 initialEyePosition = glm::vec3(0.0f, 0.0f, 0.0f);
    const glm::vec3 initialAngleAroundAxes = glm::vec3(0.0f, 0.0f, 0.0f);

    const float nearPlane = .1f;
    const float farPlane = 2000000.0f;
    const float eyeMovementPerMouseDelta = 0.025f;
    const float eyeRotationPerMouseDelta = 60.f;
    const float eyeMovementPerWheelAngle = .1f;
    const int mouseMaxDrag = 500;
    const float fov = 60; // Field of view (angle from Display bottom to top) in degrees

    glm::vec3 angleAroundAxes = initialAngleAroundAxes; // Camera direction in degrees
    glm::vec3 eyePosition = initialEyePosition; // Camera coordinates
    float w = 400, h = 400;         // Display width and height.

public:
    PerspectiveCamera();

    void setWH(float w, float h) override;

    void processMoveRequest(const int &deltaX, const int &deltaY) override;

    void processRotateRequest(const int &deltaX, const int &deltaY, const bool &thirdAxis) override;

    void processZoomRequest(const int &deltaWheelAngle) override;

    glm::mat4 modelViewMatrix() const override;

    glm::mat4 projectionMatrix() const override;
};


#endif //RT3_PerspectiveCamera_H
