#include "floatpoint.h"

#include "transformationmatrix.h"

#include "common.h"

/*#include "config.h"
#include "FloatPoint.h"

#include "TransformationMatrix.h"
#include "FloatConversion.h"
#include "IntPoint.h"*/

#include <cmath>

FloatPoint::FloatPoint(const IntPoint& p) : m_x(p.x()), m_y(p.y())
{
}

void FloatPoint::normalize()
{
    float tempLength = length();

    if (tempLength) {
        m_x /= tempLength;
        m_y /= tempLength;
    }
}

float FloatPoint::length() const
{
    return sqrtf(lengthSquared());
}

FloatPoint FloatPoint::matrixTransform(const AffineTransform& transform) const
{
    double newX, newY;
    transform.map(static_cast<double>(m_x), static_cast<double>(m_y), newX, newY);
    return narrowPrecision(newX, newY);
}

FloatPoint FloatPoint::matrixTransform(const TransformationMatrix& transform) const
{
    double newX, newY;
    transform.map(static_cast<double>(m_x), static_cast<double>(m_y), newX, newY);
    return narrowPrecision(newX, newY);
}

FloatPoint FloatPoint::narrowPrecision(double x, double y)
{
    return FloatPoint(narrowPrecisionToFloat(x), narrowPrecisionToFloat(y));
}

//////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/FloatPointQt.cpp
FloatPoint::FloatPoint(const QPointF& p)
    : m_x(p.x())
    , m_y(p.y())
{
}

FloatPoint::operator QPointF() const
{
    return QPointF(m_x, m_y);
}

