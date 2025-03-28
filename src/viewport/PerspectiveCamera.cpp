// This class probably does not work anymore
#include "PerspectiveCamera.h"

PerspectiveCamera::PerspectiveCamera() = default;

QMatrix4x4 PerspectiveCamera::modelViewMatrix() const {
    QMatrix4x4 rotationMatrixAroundX;
    rotationMatrixAroundX.rotate(angleAroundAxes.x(), axisX);
    QMatrix4x4 rotationMatrixAroundY;
    rotationMatrixAroundY.rotate(angleAroundAxes.y(), axisY);
    QMatrix4x4 rotationMatrixAroundZ;
    rotationMatrixAroundZ.rotate(angleAroundAxes.z(), axisZ);
    QMatrix4x4 rotationMatrix = rotationMatrixAroundX * rotationMatrixAroundY * rotationMatrixAroundZ;
    rotationMatrix.translate(eyePosition);
    return rotationMatrix;
}

QMatrix4x4 PerspectiveCamera::projectionMatrix() const {
    QMatrix4x4 ret;
    ret.perspective(fov,w/h,nearPlane,farPlane);
    return ret;
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
        angleAroundAxes.setY(angleAroundAxes.y() + deltaAngleX * eyeRotationPerMouseDelta);
    } else {
        angleAroundAxes.setZ(angleAroundAxes.z() + deltaAngleX * eyeRotationPerMouseDelta);
    }
    angleAroundAxes.setX(angleAroundAxes.x() + deltaAngleY * eyeRotationPerMouseDelta);
}

void PerspectiveCamera::processMoveRequest(const int & deltaX, const int & deltaY){
    if (deltaX < -mouseMaxDrag || deltaX > +mouseMaxDrag || deltaY < -mouseMaxDrag || deltaY > +mouseMaxDrag){
        return;
    }

    QMatrix4x4 rotationMatrixAroundY;
    rotationMatrixAroundY.rotate(-angleAroundAxes.y(), axisY);
    QMatrix4x4 rotationMatrixAroundX;
    rotationMatrixAroundX.rotate(-angleAroundAxes.x(), axisX);
    QMatrix4x4 rotationMatrix = rotationMatrixAroundY * rotationMatrixAroundX;

    QVector3D PerspectiveCameraRightDirection = rotationMatrix.map(axisX);
    eyePosition += float(deltaX) * eyeMovementPerMouseDelta * PerspectiveCameraRightDirection;

    QVector3D PerspectiveCameraUpDirection = rotationMatrix.map(axisY);
    eyePosition += float(-deltaY) * eyeMovementPerMouseDelta * PerspectiveCameraUpDirection;
}

void PerspectiveCamera::processZoomRequest(const int & deltaWheelAngle) {
    QMatrix4x4 rotationMatrixAroundY;
    rotationMatrixAroundY.rotate(-angleAroundAxes.y(), axisY);
    QMatrix4x4 rotationMatrixAroundX;
    rotationMatrixAroundX.rotate(-angleAroundAxes.x(), axisX);
    QMatrix4x4 rotationMatrix = rotationMatrixAroundY * rotationMatrixAroundX;

    QVector3D PerspectiveCameraForwardDirection = rotationMatrix.map(-axisZ);

    eyePosition += -float(deltaWheelAngle) * eyeMovementPerWheelAngle * PerspectiveCameraForwardDirection;
}
