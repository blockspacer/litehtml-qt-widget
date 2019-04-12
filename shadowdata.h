#pragma once

#include "color.h"
//#include <wtf/FastAllocBase.h>

class FloatRect;
class IntRect;

enum ShadowStyle { Normal, Inset };

// This struct holds information about shadows for the text-shadow and box-shadow properties.

class ShadowData /*: public FastAllocBase*/ {
public:
    ShadowData()
        : m_x(0)
        , m_y(0)
        , m_blur(0)
        , m_spread(0)
        , m_style(Normal)
        , m_next(0)
    {
    }

    ShadowData(int x, int y, int blur, int spread, ShadowStyle style, const Color& color)
        : m_x(x)
        , m_y(y)
        , m_blur(blur)
        , m_spread(spread)
        , m_style(style)
        , m_color(color)
        , m_next(0)
    {
    }

    ShadowData(const ShadowData& o);
    ~ShadowData() { delete m_next; }

    bool operator==(const ShadowData& o) const;
    bool operator!=(const ShadowData& o) const
    {
        return !(*this == o);
    }

    int x() const { return m_x; }
    int y() const { return m_y; }
    int blur() const { return m_blur; }
    int spread() const { return m_spread; }
    ShadowStyle style() const { return m_style; }
    const Color& color() const { return m_color; }

    const ShadowData* next() const { return m_next; }
    void setNext(ShadowData* shadow) { m_next = shadow; }

    void adjustRectForShadow(IntRect&, int additionalOutlineSize = 0) const;
    void adjustRectForShadow(FloatRect&, int additionalOutlineSize = 0) const;

private:
    int m_x;
    int m_y;
    int m_blur;
    int m_spread;
    ShadowStyle m_style;
    Color m_color;
    ShadowData* m_next;
};
