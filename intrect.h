#pragma once

/*#include "IntPoint.h"
#include <wtf/Vector.h>
*/

#include <common.h>
//#include "vector.h"

//#include "floatpoint.h"
//#include "floatrect.h"
#include "intpoint.h"
#include "intsize.h"
//#include "floatsize.h"
//#include "intrect.h"

QT_BEGIN_NAMESPACE
class QRect;
QT_END_NAMESPACE


class FloatRect;

class IntRect {
public:
    IntRect() { }
    IntRect(const IntPoint& location, const IntSize& size)
        : m_location(location), m_size(size) { }
    IntRect(int x, int y, int width, int height)
        : m_location(IntPoint(x, y)), m_size(IntSize(width, height)) { }

    explicit IntRect(const FloatRect& rect); // don't do this implicitly since it's lossy

    IntPoint location() const { return m_location; }
    IntSize size() const { return m_size; }

    void setLocation(const IntPoint& location) { m_location = location; }
    void setSize(const IntSize& size) { m_size = size; }

    int x() const { return m_location.x(); }
    int y() const { return m_location.y(); }
    int width() const { return m_size.width(); }
    int height() const { return m_size.height(); }

    void setX(int x) { m_location.setX(x); }
    void setY(int y) { m_location.setY(y); }
    void setWidth(int width) { m_size.setWidth(width); }
    void setHeight(int height) { m_size.setHeight(height); }

    // Be careful with these functions.  The point is considered to be to the right and below.  These are not
    // substitutes for right() and bottom().
    IntPoint topLeft() const { return m_location; }
    IntPoint topRight() const { return IntPoint(right() - 1, y()); }
    IntPoint bottomLeft() const { return IntPoint(x(), bottom() - 1); }
    IntPoint bottomRight() const { return IntPoint(right() - 1, bottom() - 1); }

    bool isEmpty() const { return m_size.isEmpty(); }

    int right() const { return x() + width(); }
    int bottom() const { return y() + height(); }

    // NOTE: The result is rounded to integer values, and thus may be not the exact
    // center point.
    IntPoint center() const { return IntPoint(x() + width() / 2, y() + height() / 2); }

    void move(const IntSize& s) { m_location += s; }
    void move(int dx, int dy) { m_location.move(dx, dy); }

    bool intersects(const IntRect&) const;
    bool contains(const IntRect&) const;

    // This checks to see if the rect contains x,y in the traditional sense.
    // Equivalent to checking if the rect contains a 1x1 rect below and to the right of (px,py).
    bool contains(int px, int py) const
        { return px >= x() && px < right() && py >= y() && py < bottom(); }
    bool contains(const IntPoint& point) const { return contains(point.x(), point.y()); }

    void intersect(const IntRect&);
    void unite(const IntRect&);

    void inflateX(int dx)
    {
        m_location.setX(m_location.x() - dx);
        m_size.setWidth(m_size.width() + dx + dx);
    }
    void inflateY(int dy)
    {
        m_location.setY(m_location.y() - dy);
        m_size.setHeight(m_size.height() + dy + dy);
    }
    void inflate(int d) { inflateX(d); inflateY(d); }
    void scale(float s);


    IntRect(const QRect&);
    operator QRect() const;

private:
    IntPoint m_location;
    IntSize m_size;
};

inline IntRect intersection(const IntRect& a, const IntRect& b)
{
    IntRect c = a;
    c.intersect(b);
    return c;
}

inline IntRect unionRect(const IntRect& a, const IntRect& b)
{
    IntRect c = a;
    c.unite(b);
    return c;
}

IntRect unionRect(const QVector<IntRect>&);

inline bool operator==(const IntRect& a, const IntRect& b)
{
    return a.location() == b.location() && a.size() == b.size();
}

inline bool operator!=(const IntRect& a, const IntRect& b)
{
    return a.location() != b.location() || a.size() != b.size();
}
