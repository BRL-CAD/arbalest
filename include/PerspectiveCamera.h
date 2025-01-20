//
// Created by Sadeep on 09-Jun.
// This class probably does not work anymore

#ifndef RT3_PerspectiveCamera_H
#define RT3_PerspectiveCamera_H


#include "Camera.h"


class PerspectiveCamera : public Camera {
private:
    const QVector3D initialEyePosition = QVector3D(0.0f, 0.0f, 0.0f);
    const QVector3D initialAngleAroundAxes = QVector3D(0.0f, 0.0f, 0.0f);

    const float nearPlane = .1f;
    const float farPlane = 2000000.0f;
    const float eyeMovementPerMouseDelta = 0.025f;
    const float eyeRotationPerMouseDelta = 60.f;
    const float eyeMovementPerWheelAngle = .1f;
    const int mouseMaxDrag = 500;
    const float fov = 60; // Field of view (angle from ArbDisplay bottom to top) in degrees

    QVector3D angleAroundAxes = initialAngleAroundAxes; // Camera direction in degrees
    QVector3D eyePosition = initialEyePosition; // Camera coordinates
    float w = 400, h = 400;         // ArbDisplay width and height.

public:
    PerspectiveCamera();

    void setWH(float w, float h) override;

    void processMoveRequest(const int &deltaX, const int &deltaY) override;

    void processRotateRequest(const int &deltaX, const int &deltaY, const bool &thirdAxis) override;

    void processZoomRequest(const int &deltaWheelAngle) override;

    QMatrix4x4 modelViewMatrix() const override;

    QMatrix4x4 projectionMatrix() const override;
};


#endif //RT3_PerspectiveCamera_H
