#include "shadowdata.h"

#include "common.h"

//#include "config.h"
#include "shadowdata.h"

#include "floatrect.h"
#include "intrect.h"

using namespace std;

ShadowData::ShadowData(const ShadowData& o)
    : m_x(o.m_x)
    , m_y(o.m_y)
    , m_blur(o.m_blur)
    , m_spread(o.m_spread)
    , m_style(o.m_style)
    , m_color(o.m_color)
{
    m_next = o.m_next ? new ShadowData(*o.m_next) : 0;
}

bool ShadowData::operator==(const ShadowData& o) const
{
    if ((m_next && !o.m_next) || (!m_next && o.m_next) ||
        (m_next && o.m_next && *m_next != *o.m_next))
        return false;

    return m_x == o.m_x && m_y == o.m_y && m_blur == o.m_blur && m_spread == o.m_spread && m_style == o.m_style && m_color == o.m_color;
}

static inline void calculateShadowExtent(const ShadowData* shadow, int additionalOutlineSize, int& shadowLeft, int& shadowRight, int& shadowTop, int& shadowBottom)
{
    do {
        int blurAndSpread = shadow->blur() + shadow->spread() + additionalOutlineSize;
        if (shadow->style() == Normal) {
            shadowLeft = min(shadow->x() - blurAndSpread, shadowLeft);
            shadowRight = max(shadow->x() + blurAndSpread, shadowRight);
            shadowTop = min(shadow->y() - blurAndSpread, shadowTop);
            shadowBottom = max(shadow->y() + blurAndSpread, shadowBottom);
        }

        shadow = shadow->next();
    } while (shadow);
}

void ShadowData::adjustRectForShadow(IntRect& rect, int additionalOutlineSize) const
{
    int shadowLeft = 0;
    int shadowRight = 0;
    int shadowTop = 0;
    int shadowBottom = 0;
    calculateShadowExtent(this, additionalOutlineSize, shadowLeft, shadowRight, shadowTop, shadowBottom);

    rect.move(shadowLeft, shadowTop);
    rect.setWidth(rect.width() - shadowLeft + shadowRight);
    rect.setHeight(rect.height() - shadowTop + shadowBottom);
}

void ShadowData::adjustRectForShadow(FloatRect& rect, int additionalOutlineSize) const
{
    int shadowLeft = 0;
    int shadowRight = 0;
    int shadowTop = 0;
    int shadowBottom = 0;
    calculateShadowExtent(this, additionalOutlineSize, shadowLeft, shadowRight, shadowTop, shadowBottom);

    rect.move(shadowLeft, shadowTop);
    rect.setWidth(rect.width() - shadowLeft + shadowRight);
    rect.setHeight(rect.height() - shadowTop + shadowBottom);
}
