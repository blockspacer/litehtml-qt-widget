#include "floatrect.h"

#include "common.h"

/*#include "config.h"
#include "FloatRect.h"

#include "FloatConversion.h"
#include "IntRect.h"*/

#include <algorithm>
#include <limits>
#include <cmath>

#include "floatpoint.h"
#include "floatrect.h"
#include "intpoint.h"
#include "intsize.h"
#include "floatsize.h"
#include "intrect.h"

#include <QRectF>

using std::max;
using std::min;

FloatRect::FloatRect(const IntRect& r) : m_location(r.location()), m_size(r.size())
{
}

FloatRect FloatRect::narrowPrecision(double x, double y, double width, double height)
{
    return FloatRect(narrowPrecisionToFloat(x), narrowPrecisionToFloat(y), narrowPrecisionToFloat(width), narrowPrecisionToFloat(height));
}

bool FloatRect::intersects(const FloatRect& other) const
{
    // Checking emptiness handles negative widths as well as zero.
    return !isEmpty() && !other.isEmpty()
        && x() < other.right() && other.x() < right()
        && y() < other.bottom() && other.y() < bottom();
}

bool FloatRect::contains(const FloatRect& other) const
{
    return x() <= other.x() && right() >= other.right()
        && y() <= other.y() && bottom() >= other.bottom();
}

void FloatRect::intersect(const FloatRect& other)
{
    float l = max(x(), other.x());
    float t = max(y(), other.y());
    float r = min(right(), other.right());
    float b = min(bottom(), other.bottom());

    // Return a clean empty rectangle for non-intersecting cases.
    if (l >= r || t >= b) {
        l = 0;
        t = 0;
        r = 0;
        b = 0;
    }

    setLocationAndSizeFromEdges(l, t, r, b);
}

void FloatRect::unite(const FloatRect& other)
{
    // Handle empty special cases first.
    if (other.isEmpty())
        return;
    if (isEmpty()) {
        *this = other;
        return;
    }

    float l = min(x(), other.x());
    float t = min(y(), other.y());
    float r = max(right(), other.right());
    float b = max(bottom(), other.bottom());

    setLocationAndSizeFromEdges(l, t, r, b);
}

void FloatRect::scale(float sx, float sy)
{
    m_location.setX(x() * sx);
    m_location.setY(y() * sy);
    m_size.setWidth(width() * sx);
    m_size.setHeight(height() * sy);
}

void FloatRect::fitToPoints(const FloatPoint& p0, const FloatPoint& p1)
{
    float left = min(p0.x(), p1.x());
    float top = min(p0.y(), p1.y());
    float right = max(p0.x(), p1.x());
    float bottom = max(p0.y(), p1.y());

    setLocationAndSizeFromEdges(left, top, right, bottom);
}

namespace {
// Helpers for 3- and 4-way max and min.

template <typename T>
T min3(const T& v1, const T& v2, const T& v3)
{
    return min(min(v1, v2), v3);
}

template <typename T>
T max3(const T& v1, const T& v2, const T& v3)
{
    return max(max(v1, v2), v3);
}

template <typename T>
T min4(const T& v1, const T& v2, const T& v3, const T& v4)
{
    return min(min(v1, v2), min(v3, v4));
}

template <typename T>
T max4(const T& v1, const T& v2, const T& v3, const T& v4)
{
    return max(max(v1, v2), max(v3, v4));
}

} // anonymous namespace

void FloatRect::fitToPoints(const FloatPoint& p0, const FloatPoint& p1, const FloatPoint& p2)
{
    float left = min3(p0.x(), p1.x(), p2.x());
    float top = min3(p0.y(), p1.y(), p2.y());
    float right = max3(p0.x(), p1.x(), p2.x());
    float bottom = max3(p0.y(), p1.y(), p2.y());

    setLocationAndSizeFromEdges(left, top, right, bottom);
}

void FloatRect::fitToPoints(const FloatPoint& p0, const FloatPoint& p1, const FloatPoint& p2, const FloatPoint& p3)
{
    float left = min4(p0.x(), p1.x(), p2.x(), p3.x());
    float top = min4(p0.y(), p1.y(), p2.y(), p3.y());
    float right = max4(p0.x(), p1.x(), p2.x(), p3.x());
    float bottom = max4(p0.y(), p1.y(), p2.y(), p3.y());

    setLocationAndSizeFromEdges(left, top, right, bottom);
}

static inline int safeFloatToInt(float x)
{
    static const int s_intMax = std::numeric_limits<int>::max();
    static const int s_intMin = std::numeric_limits<int>::min();

    if (x >= static_cast<float>(s_intMax))
        return s_intMax;
    if (x < static_cast<float>(s_intMin))
        return s_intMin;
    return static_cast<int>(x);
}

IntRect enclosingIntRect(const FloatRect& rect)
{
    float left = floorf(rect.x());
    float top = floorf(rect.y());
    float width = ceilf(rect.right()) - left;
    float height = ceilf(rect.bottom()) - top;
    return IntRect(safeFloatToInt(left), safeFloatToInt(top),
                   safeFloatToInt(width), safeFloatToInt(height));
}

FloatRect mapRect(const FloatRect& r, const FloatRect& srcRect, const FloatRect& destRect)
{
    if (srcRect.width() == 0 || srcRect.height() == 0)
        return FloatRect();

    float widthScale = destRect.width() / srcRect.width();
    float heightScale = destRect.height() / srcRect.height();
    return FloatRect(destRect.x() + (r.x() - srcRect.x()) * widthScale,
                     destRect.y() + (r.y() - srcRect.y()) * heightScale,
                     r.width() * widthScale, r.height() * heightScale);
}

/////////////////
// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/FloatRectQt.cpp

FloatRect::FloatRect(const QRectF& r)
    : m_location(r.topLeft())
    , m_size(r.width()
    , r.height())
{
}

FloatRect::operator QRectF() const
{
    return QRectF(x(), y(), width(), height());
}

FloatRect FloatRect::normalized() const
{
    FloatRect normalizedRect = *this;

    if (width() < 0) {
        normalizedRect.setX(x() + width());
        normalizedRect.setWidth(-width());
    }
    if (height() < 0) {
        normalizedRect.setY(y() + height());
        normalizedRect.setHeight(-height());
    }
    return normalizedRect;
}
