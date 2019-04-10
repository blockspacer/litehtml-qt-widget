#pragma once


#include "common.h"
#include "intpoint.h"
#include "floatsize.h"

/*#include "IntPoint.h"
#include <wtf/MathExtras.h>*/

#include "qglobal.h"
QT_BEGIN_NAMESPACE
class QPointF;
QT_END_NAMESPACE

class AffineTransform;
class TransformationMatrix;
//class IntPoint;

class FloatPoint {
public:
    FloatPoint() : m_x(0), m_y(0) { }
    FloatPoint(float x, float y) : m_x(x), m_y(y) { }
    FloatPoint(const IntPoint&);


    static FloatPoint zero() { return FloatPoint(); }

    static FloatPoint narrowPrecision(double x, double y);

    float x() const { return m_x; }
    float y() const { return m_y; }

    void setX(float x) { m_x = x; }
    void setY(float y) { m_y = y; }
    void set(float x, float y)
    {
        m_x = x;
        m_y = y;
    }
    void move(float dx, float dy)
    {
        m_x += dx;
        m_y += dy;
    }
    void scale(float sx, float sy)
    {
        m_x *= sx;
        m_y *= sy;
    }

    void normalize();

    float dot(const FloatPoint& a) const
    {
        return m_x * a.x() + m_y * a.y();
    }

    float length() const;
    float lengthSquared() const
    {
        return m_x * m_x + m_y * m_y;
    }

    FloatPoint(const QPointF&);
    operator QPointF() const;

    FloatPoint matrixTransform(const TransformationMatrix&) const;
    FloatPoint matrixTransform(const AffineTransform&) const;

private:
    float m_x, m_y;
};


inline FloatPoint& operator+=(FloatPoint& a, const FloatSize& b)
{
    a.move(b.width(), b.height());
    return a;
}

inline FloatPoint& operator+=(FloatPoint& a, const FloatPoint& b)
{
    a.move(b.x(), b.y());
    return a;
}

inline FloatPoint& operator-=(FloatPoint& a, const FloatSize& b)
{
    a.move(-b.width(), -b.height());
    return a;
}

inline FloatPoint operator+(const FloatPoint& a, const FloatSize& b)
{
    return FloatPoint(a.x() + b.width(), a.y() + b.height());
}

inline FloatPoint operator+(const FloatPoint& a, const FloatPoint& b)
{
    return FloatPoint(a.x() + b.x(), a.y() + b.y());
}

inline FloatSize operator-(const FloatPoint& a, const FloatPoint& b)
{
    return FloatSize(a.x() - b.x(), a.y() - b.y());
}

inline FloatPoint operator-(const FloatPoint& a, const FloatSize& b)
{
    return FloatPoint(a.x() - b.width(), a.y() - b.height());
}

inline bool operator==(const FloatPoint& a, const FloatPoint& b)
{
    return a.x() == b.x() && a.y() == b.y();
}

inline bool operator!=(const FloatPoint& a, const FloatPoint& b)
{
    return a.x() != b.x() || a.y() != b.y();
}

inline float operator*(const FloatPoint& a, const FloatPoint& b)
{
    // dot product
    return a.dot(b);
}

inline IntPoint roundedIntPoint(const FloatPoint& p)
{
    return IntPoint(static_cast<int>(roundf(p.x())), static_cast<int>(roundf(p.y())));
}

