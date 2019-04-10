#include "floatquad.h"

#include "common.h"

#include "floatquad.h"

#include <algorithm>

using std::max;
using std::min;

static inline float min4(float a, float b, float c, float d)
{
    return min(min(a, b), min(c, d));
}

static inline float max4(float a, float b, float c, float d)
{
    return max(max(a, b), max(c, d));
}

inline float dot(const FloatSize& a, const FloatSize& b)
{
    return a.width() * b.width() + a.height() * b.height();
}

inline bool isPointInTriangle(const FloatPoint& p, const FloatPoint& t1, const FloatPoint& t2, const FloatPoint& t3)
{
    // Compute vectors
    FloatSize v0 = t3 - t1;
    FloatSize v1 = t2 - t1;
    FloatSize v2 = p - t1;

    // Compute dot products
    float dot00 = dot(v0, v0);
    float dot01 = dot(v0, v1);
    float dot02 = dot(v0, v2);
    float dot11 = dot(v1, v1);
    float dot12 = dot(v1, v2);

    // Compute barycentric coordinates
    float invDenom = 1.0f / (dot00 * dot11 - dot01 * dot01);
    float u = (dot11 * dot02 - dot01 * dot12) * invDenom;
    float v = (dot00 * dot12 - dot01 * dot02) * invDenom;

    // Check if point is in triangle
    return (u >= 0) && (v >= 0) && (u + v <= 1);
}

FloatRect FloatQuad::boundingBox() const
{
    float left   = min4(m_p1.x(), m_p2.x(), m_p3.x(), m_p4.x());
    float top    = min4(m_p1.y(), m_p2.y(), m_p3.y(), m_p4.y());

    float right  = max4(m_p1.x(), m_p2.x(), m_p3.x(), m_p4.x());
    float bottom = max4(m_p1.y(), m_p2.y(), m_p3.y(), m_p4.y());

    return FloatRect(left, top, right - left, bottom - top);
}

bool FloatQuad::isRectilinear() const
{
    return (m_p1.x() == m_p2.x() && m_p2.y() == m_p3.y() && m_p3.x() == m_p4.x() && m_p4.y() == m_p1.y())
        || (m_p1.y() == m_p2.y() && m_p2.x() == m_p3.x() && m_p3.y() == m_p4.y() && m_p4.x() == m_p1.x());
}

bool FloatQuad::containsPoint(const FloatPoint& p) const
{
    return isPointInTriangle(p, m_p1, m_p2, m_p3) || isPointInTriangle(p, m_p1, m_p3, m_p4);
}

// Note that we only handle convex quads here.
bool FloatQuad::containsQuad(const FloatQuad& other) const
{
    return containsPoint(other.p1()) && containsPoint(other.p2()) && containsPoint(other.p3()) && containsPoint(other.p4());
}
