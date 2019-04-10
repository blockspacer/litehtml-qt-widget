#pragma once

#include "floatpoint.h"

class FloatPoint3D {
public:
    FloatPoint3D()
        : m_x(0)
        , m_y(0)
        , m_z(0)
    {
    }

    FloatPoint3D(float x, float y, float z)
        : m_x(x)
        , m_y(y)
        , m_z(z)
    {
    }

    FloatPoint3D(const FloatPoint& p)
        : m_x(p.x())
        , m_y(p.y())
        , m_z(0)
    {
    }

    FloatPoint3D(const FloatPoint3D& p)
        : m_x(p.x())
        , m_y(p.y())
        , m_z(p.z())
    {
    }

    float x() const { return m_x; }
    void setX(float x) { m_x = x; }

    float y() const { return m_y; }
    void setY(float y) { m_y = y; }

    float z() const { return m_z; }
    void setZ(float z) { m_z = z; }
    void set(float x, float y, float z)
    {
        m_x = x;
        m_y = y;
        m_z = z;
    }
    void move(float dx, float dy, float dz)
    {
        m_x += dx;
        m_y += dy;
        m_z += dz;
    }
    void scale(float sx, float sy, float sz)
    {
        m_x *= sx;
        m_y *= sy;
        m_z *= sz;
    }

    void normalize();

    float dot(const FloatPoint3D& a) const
    {
        return m_x * a.x() + m_y * a.y() + m_z * a.z();
    }

    // Sets this FloatPoint3D to the cross product of the passed two.
    // It is safe for "this" to be the same as either or both of the
    // arguments.
    void cross(const FloatPoint3D& a, const FloatPoint3D& b)
    {
        float x = a.y() * b.z() - a.z() * b.y();
        float y = a.z() * b.x() - a.x() * b.z();
        float z = a.x() * b.y() - a.y() * b.x();
        m_x = x;
        m_y = y;
        m_z = z;
    }

    // Convenience function returning "this cross point" as a
    // stack-allocated result.
    FloatPoint3D cross(const FloatPoint3D& point) const
    {
        FloatPoint3D result;
        result.cross(*this, point);
        return result;
    }

    float length() const;
    float lengthSquared() const { return this->dot(*this); }

private:
    float m_x;
    float m_y;
    float m_z;
};

inline FloatPoint3D& operator +=(FloatPoint3D& a, const FloatPoint3D& b)
{
    a.move(b.x(), b.y(), b.z());
    return a;
}

inline FloatPoint3D& operator -=(FloatPoint3D& a, const FloatPoint3D& b)
{
    a.move(-b.x(), -b.y(), -b.z());
    return a;
}

inline FloatPoint3D operator+(const FloatPoint3D& a, const FloatPoint3D& b)
{
    return FloatPoint3D(a.x() + b.x(), a.y() + b.y(), a.z() + b.z());
}

inline FloatPoint3D operator-(const FloatPoint3D& a, const FloatPoint3D& b)
{
    return FloatPoint3D(a.x() - b.x(), a.y() - b.y(), a.z() - b.z());
}

inline bool operator==(const FloatPoint3D& a, const FloatPoint3D& b)
{
    return a.x() == b.x() && a.y() == b.y() && a.z() == b.z();
}

inline bool operator!=(const FloatPoint3D& a, const FloatPoint3D& b)
{
    return a.x() != b.x() || a.y() != b.y() || a.z() != b.z();
}

inline float operator*(const FloatPoint3D& a, const FloatPoint3D& b)
{
    // dot product
    return a.dot(b);
}
