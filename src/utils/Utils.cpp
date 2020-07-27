#include <cmath>

#include "cicommon.h"

BRLCAD::Vector3D operator+(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b)
{
    BRLCAD::Vector3D result;
    result.coordinates[0] = a.coordinates[0] + b.coordinates[0];
    result.coordinates[1] = a.coordinates[1] + b.coordinates[1];
    result.coordinates[2] = a.coordinates[2] + b.coordinates[2];
    return result;
}

BRLCAD::Vector3D operator-(const BRLCAD::Vector3D& a, const BRLCAD::Vector3D& b)
{
    BRLCAD::Vector3D result;
    result.coordinates[0] = a.coordinates[0] - b.coordinates[0];
    result.coordinates[1] = a.coordinates[1] - b.coordinates[1];
    result.coordinates[2] = a.coordinates[2] - b.coordinates[2];
    return result;
}

BRLCAD::Vector3D operator/(const BRLCAD::Vector3D& a, const int& b)
{
    BRLCAD::Vector3D result;
    result.coordinates[0] = a.coordinates[0] / b;
    result.coordinates[1] = a.coordinates[1] / b;
    result.coordinates[2] = a.coordinates[2] / b;
    return result;
}

double vector3DLength(const BRLCAD::Vector3D& a)
{
    return sqrt(a.coordinates[0] * a.coordinates[0] + a.coordinates[1] * a.coordinates[1] + a.coordinates[2] * a.coordinates[2]);
}
