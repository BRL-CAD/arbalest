//
// Created by Sadeep on 09-Jun.
//

#ifndef RT3_CAMERABK_H
#define RT3_CAMERABK_H


#include <glm/detail/type_mat4x4.hpp>


class Camera {
public:
    constexpr static const glm::vec3  axisX = glm::vec3(1.0f,0.0f,0.0f);
    constexpr static const glm::vec3  axisY = glm::vec3(0.0f,1.0f,0.0f);
    constexpr static const glm::vec3  axisZ = glm::vec3(0.0f,0.0f,1.0f);

    virtual void setWH(float w, float h) = 0;
    virtual void processMoveRequest(const int &deltaX, const int &deltaY) = 0;
    virtual void processRotateRequest(const int & deltaX, const int & deltaY, const bool& thirdAxis) = 0;
    virtual void processZoomRequest(const int & deltaWheelAngle) = 0;

    virtual glm::mat4 modelViewMatrix() const = 0;
    virtual glm::mat4 projectionMatrix() const = 0;

};


#endif //RT3_CAMERABK_H
