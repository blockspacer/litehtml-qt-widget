#include "gradient.h"

#include <QGradient>
#include <QPainter>

#include "common.h"
//#include "Gradient.h"

#include "color.h"
#include "floatrect.h"
//#include <wtf/UnusedParam.h>

Gradient::Gradient(const FloatPoint& p0, const FloatPoint& p1)
    : m_radial(false)
    , m_p0(p0)
    , m_p1(p1)
    , m_r0(0)
    , m_r1(0)
    , m_stopsSorted(false)
    , m_lastStop(0)
    , m_spreadMethod(SpreadMethodPad)
{
    platformInit();
}

Gradient::Gradient(const FloatPoint& p0, float r0, const FloatPoint& p1, float r1)
    : m_radial(true)
    , m_p0(p0)
    , m_p1(p1)
    , m_r0(r0)
    , m_r1(r1)
    , m_stopsSorted(false)
    , m_lastStop(0)
    , m_spreadMethod(SpreadMethodPad)
{
    platformInit();
}

Gradient::~Gradient()
{
    platformDestroy();
}

void Gradient::adjustParametersForTiledDrawing(IntSize& size, FloatRect& srcRect)
{
    if (m_radial)
        return;

    if (srcRect.isEmpty())
        return;

    if (m_p0.x() == m_p1.x()) {
        size.setWidth(1);
        srcRect.setWidth(1);
        srcRect.setX(0);
        return;
    }
    if (m_p0.y() != m_p1.y())
        return;

    size.setHeight(1);
    srcRect.setHeight(1);
    srcRect.setY(0);
}

void Gradient::addColorStop(float value, const Color& color)
{
    float r;
    float g;
    float b;
    float a;
    color.getRGBA(r, g, b, a);
    m_stops.append(ColorStop(value, r, g, b, a));

    m_stopsSorted = false;
    platformDestroy();
}

void Gradient::addColorStop(const Gradient::ColorStop& stop)
{
    m_stops.append(stop);

    m_stopsSorted = false;
    platformDestroy();
}

static inline bool compareStops(const Gradient::ColorStop& a, const Gradient::ColorStop& b)
{
    return a.stop < b.stop;
}

void Gradient::sortStopsIfNecessary()
{
    if (m_stopsSorted)
        return;

    m_stopsSorted = true;

    if (!m_stops.size())
        return;

    // Shortcut for the ideal case (ordered 2-stop gradient)
    if (m_stops.size() == 2 && compareStops(*m_stops.begin(), *m_stops.end()))
        return;

    std::stable_sort(m_stops.begin(), m_stops.end(), compareStops);
}

void Gradient::getColor(float value, float* r, float* g, float* b, float* a) const
{
    ASSERT(value >= 0);
    ASSERT(value <= 1);

    if (m_stops.isEmpty()) {
        *r = 0;
        *g = 0;
        *b = 0;
        *a = 0;
        return;
    }
    if (!m_stopsSorted) {
        if (m_stops.size())
            std::stable_sort(m_stops.begin(), m_stops.end(), compareStops);
        m_stopsSorted = true;
    }
    if (value <= 0 || value <= m_stops.first().stop) {
        *r = m_stops.first().red;
        *g = m_stops.first().green;
        *b = m_stops.first().blue;
        *a = m_stops.first().alpha;
        return;
    }
    if (value >= 1 || value >= m_stops.last().stop) {
        *r = m_stops.last().red;
        *g = m_stops.last().green;
        *b = m_stops.last().blue;
        *a = m_stops.last().alpha;
        return;
    }

    // Find stop before and stop after and interpolate.
    int stop = findStop(value);
    const ColorStop& lastStop = m_stops[stop];
    const ColorStop& nextStop = m_stops[stop + 1];
    float stopFraction = (value - lastStop.stop) / (nextStop.stop - lastStop.stop);
    *r = lastStop.red + (nextStop.red - lastStop.red) * stopFraction;
    *g = lastStop.green + (nextStop.green - lastStop.green) * stopFraction;
    *b = lastStop.blue + (nextStop.blue - lastStop.blue) * stopFraction;
    *a = lastStop.alpha + (nextStop.alpha - lastStop.alpha) * stopFraction;
}

int Gradient::findStop(float value) const
{
    ASSERT(value >= 0);
    ASSERT(value <= 1);
    ASSERT(m_stopsSorted);

    int numStops = m_stops.size();
    ASSERT(numStops >= 2);
    ASSERT(m_lastStop < numStops - 1);

    int i = m_lastStop;
    if (value < m_stops[i].stop)
        i = 1;
    else
        i = m_lastStop + 1;

    for (; i < numStops - 1; ++i)
        if (value < m_stops[i].stop)
            break;

    m_lastStop = i - 1;
    return m_lastStop;
}

void Gradient::setSpreadMethod(GradientSpreadMethod spreadMethod)
{
    // FIXME: Should it become necessary, allow calls to this method after m_gradient has been set.
    ASSERT(m_gradient == 0);
    m_spreadMethod = spreadMethod;
}

void Gradient::setGradientSpaceTransform(const AffineTransform& gradientSpaceTransformation)
{
    m_gradientSpaceTransformation = gradientSpaceTransformation;
    setPlatformGradientSpaceTransform(gradientSpaceTransformation);
}

//#if !PLATFORM(SKIA) && !PLATFORM(CAIRO)
void Gradient::setPlatformGradientSpaceTransform(const AffineTransform&)
{
}
//#endif

////////////////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/GradientQt.cpp


void Gradient::platformDestroy()
{
    delete m_gradient;
    m_gradient = 0;
}

QGradient* Gradient::platformGradient()
{
    if (m_gradient)
        return m_gradient;

    bool reversed = m_r0 > m_r1;

    qreal innerRadius = reversed ? m_r1 : m_r0;
    qreal outerRadius = reversed ? m_r0 : m_r1;
    QPointF center = reversed ? m_p0 : m_p1;
    QPointF focalPoint = reversed ? m_p1 : m_p0;

    if (m_radial)
        m_gradient = new QRadialGradient(center, outerRadius, focalPoint);
    else
        m_gradient = new QLinearGradient(m_p0.x(), m_p0.y(), m_p1.x(), m_p1.y());

    m_gradient->setInterpolationMode(QGradient::ComponentInterpolation);

    sortStopsIfNecessary();

    QColor stopColor;
    QVector<ColorStop>::iterator stopIterator = m_stops.begin();
    qreal lastStop(0.0);
    const qreal lastStopDiff = 0.0000001;
    while (stopIterator != m_stops.end()) {
        stopColor.setRgbF(stopIterator->red, stopIterator->green, stopIterator->blue, stopIterator->alpha);
        if (qFuzzyCompare(lastStop, qreal(stopIterator->stop)))
            lastStop = stopIterator->stop + lastStopDiff;
        else
            lastStop = stopIterator->stop;

        if (m_radial && !qFuzzyCompare(1 + outerRadius, qreal(1))) {
            lastStop = lastStop * (1.0f - innerRadius / outerRadius);
            if (!reversed)
                lastStop += innerRadius / outerRadius;
        }

        qreal stopPosition = qMin(lastStop, qreal(1.0f));

        if (m_radial && reversed)
            stopPosition = 1 - stopPosition;

        m_gradient->setColorAt(stopPosition, stopColor);
        // Keep the lastStop as orginal value, since the following stopColor depend it
        lastStop = stopIterator->stop;
        ++stopIterator;
    }

    if (m_stops.isEmpty()) {
        // The behavior of QGradient with no stops is defined differently from HTML5 spec,
        // where the latter requires the gradient to be transparent black.
        m_gradient->setColorAt(0.0, QColor(0, 0, 0, 0));
    }

    switch (m_spreadMethod) {
    case SpreadMethodPad:
        m_gradient->setSpread(QGradient::PadSpread);
        break;
    case SpreadMethodReflect:
        m_gradient->setSpread(QGradient::ReflectSpread);
        break;
    case SpreadMethodRepeat:
        m_gradient->setSpread(QGradient::RepeatSpread);
        break;
    }

    return m_gradient;
}

void Gradient::fill(GraphicsContext* context, const FloatRect& rect)
{
    context->platformContext()->fillRect(rect, *platformGradient());
}


