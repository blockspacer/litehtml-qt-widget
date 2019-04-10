#pragma once


#include "floatpoint.h"
#include "floatrect.h"
#include "intrect.h"

// A FloatQuad is a collection of 4 points, often representing the result of
// mapping a rectangle through transforms. When initialized from a rect, the
// points are in clockwise order from top left.
class FloatQuad {
public:
    FloatQuad()
    {
    }

    FloatQuad(const FloatPoint& p1, const FloatPoint& p2, const FloatPoint& p3, const FloatPoint& p4)
        : m_p1(p1)
        , m_p2(p2)
        , m_p3(p3)
        , m_p4(p4)
    {
    }

    FloatQuad(const FloatRect& inRect)
        : m_p1(inRect.location())
        , m_p2(inRect.right(), inRect.y())
        , m_p3(inRect.right(), inRect.bottom())
        , m_p4(inRect.x(), inRect.bottom())
    {
    }

    FloatPoint p1() const { return m_p1; }
    FloatPoint p2() const { return m_p2; }
    FloatPoint p3() const { return m_p3; }
    FloatPoint p4() const { return m_p4; }

    void setP1(const FloatPoint& p) { m_p1 = p; }
    void setP2(const FloatPoint& p) { m_p2 = p; }
    void setP3(const FloatPoint& p) { m_p3 = p; }
    void setP4(const FloatPoint& p) { m_p4 = p; }

    // isEmpty tests that the bounding box is empty. This will not identify
    // "slanted" empty quads.
    bool isEmpty() const { return boundingBox().isEmpty(); }

    // Tests whether this quad can be losslessly represented by a FloatRect,
    // that is, if two edges are parallel to the x-axis and the other two
    // are parallel to the y-axis. If this method returns true, the
    // corresponding FloatRect can be retrieved with boundingBox().
    bool isRectilinear() const;

    // Tests whether the given point is inside, or on an edge or corner of this quad.
    bool containsPoint(const FloatPoint&) const;

    // Tests whether the four corners of other are inside, or coincident with the sides of this quad.
    // Note that this only works for convex quads, but that includes all quads that originate
    // from transformed rects.
    bool containsQuad(const FloatQuad&) const;

    FloatRect boundingBox() const;
    IntRect enclosingBoundingBox() const
    {
        return enclosingIntRect(boundingBox());
    }

    void move(const FloatSize& offset)
    {
        m_p1 += offset;
        m_p2 += offset;
        m_p3 += offset;
        m_p4 += offset;
    }

    void move(float dx, float dy)
    {
        m_p1.move(dx, dy);
        m_p2.move(dx, dy);
        m_p3.move(dx, dy);
        m_p4.move(dx, dy);
    }

private:
    FloatPoint m_p1;
    FloatPoint m_p2;
    FloatPoint m_p3;
    FloatPoint m_p4;
};

inline FloatQuad& operator+=(FloatQuad& a, const FloatSize& b)
{
    a.move(b);
    return a;
}

inline FloatQuad& operator-=(FloatQuad& a, const FloatSize& b)
{
    a.move(-b.width(), -b.height());
    return a;
}

inline bool operator==(const FloatQuad& a, const FloatQuad& b)
{
    return a.p1() == b.p1() &&
           a.p2() == b.p2() &&
           a.p3() == b.p3() &&
           a.p4() == b.p4();
}

inline bool operator!=(const FloatQuad& a, const FloatQuad& b)
{
    return a.p1() != b.p1() ||
           a.p2() != b.p2() ||
           a.p3() != b.p3() ||
           a.p4() != b.p4();
}
