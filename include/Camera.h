//
// Created by Sadeep on 09-Jun.
//

#ifndef RT3_CAMERA_H
#define RT3_CAMERA_H


#include <glm/detail/type_mat4x4.hpp>

const auto axisX = glm::vec3(1.0,0.0,0.0);
const auto axisY = glm::vec3(0.0,1.0,0.0);
const auto axisZ = glm::vec3(0.0,0.0,1.0);

class Camera {
public:
    Camera();

    float w = 400, h = 400;         // Display width and height.
    void setWH(float w, float h);


    glm::vec3 eyePosition = glm::vec3(0,-50,-50); // Camera coordinates
    float angleAroundY = 0, angleAroundX = 0; // Camera direction in degrees

    float nearPlane = .01, farPlane = 2000000.0f; // For perspective projection

    float fov=60; // Field of view (angle from Display bottom to top) in degrees

    float eyeMovementPerWheelAngle = .1; // move forward/backward when wheel rotate
    float eyeMovementPerMouseMove = .025; // move right/left and up/down (relative to current rotation) when mouse drag

    glm::mat4 modelViewMatrix() const;
    glm::mat4 projectionMatrix() const;

    void processMouseDrag(int deltaX, int deltaY, bool rotate);

    void processMouseWheel(int deltaWheelAngle);
};


#endif //RT3_CAMERA_H
