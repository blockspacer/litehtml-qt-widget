#include "floatpoint3d.h"

#include <cmath>

void FloatPoint3D::normalize()
{
    float tempLength = length();

    if (tempLength) {
        m_x /= tempLength;
        m_y /= tempLength;
        m_z /= tempLength;
    }
}

float FloatPoint3D::length() const
{
    return sqrtf(lengthSquared());
}
