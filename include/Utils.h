#ifndef UTILS_ARBALEST_H
#define UTILS_ARBALEST_H

#include "brlcad/cicommon.h"

BRLCAD::Vector3D operator+(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator-(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b);
BRLCAD::Vector3D operator/(const BRLCAD::Vector3D& a, const int& b);
double vector3DLength(const BRLCAD::Vector3D& a);


#endif // UTILS_ARBALEST_H