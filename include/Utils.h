#ifndef UTILS_ARBALEST_H
#define UTILS_ARBALEST_H

#include "brlcad/cicommon.h"

BRLCAD::Vector3D operator+(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator-(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator/(const BRLCAD::Vector3D& a, const int& b);
double vector3DLength(const BRLCAD::Vector3D& a);

QString breakStringAtCaps(const QString& in);


struct ColorInfo {
    float red, green, blue;
    bool hasColor;
};

#endif // UTILS_ARBALEST_H