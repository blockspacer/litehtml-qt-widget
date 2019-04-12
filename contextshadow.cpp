#include "contextshadow.h"

/*#include "config.h"
#include "ContextShadow.h"

#include <wtf/MathExtras.h>
#include <wtf/Noncopyable.h>

using WTF::min;
using WTF::max;*/

ContextShadow::ContextShadow()
    : m_type(NoShadow)
    , m_blurDistance(0)
{
}

ContextShadow::ContextShadow(const Color& color, float radius, const FloatSize& offset)
    : m_color(color)
    , m_blurDistance(round(radius))
    , m_offset(offset)
{
    // See comments in http://webkit.org/b/40793, it seems sensible
    // to follow Skia's limit of 128 pixels of blur radius
    m_blurDistance = std::min(m_blurDistance, 128);

    // The type of shadow is decided by the blur radius, shadow offset, and shadow color.
    if (!m_color.isValid() || !color.alpha()) {
        // Can't paint the shadow with invalid or invisible color.
        m_type = NoShadow;
    } else if (radius > 0) {
        // Shadow is always blurred, even the offset is zero.
        m_type = BlurShadow;
    } else if (!m_offset.width() && !m_offset.height()) {
        // Without blur and zero offset means the shadow is fully hidden.
        m_type = NoShadow;
    } else {
        m_type = SolidShadow;
    }
}

void ContextShadow::clear()
{
    m_type = NoShadow;
    m_color = Color();
    m_blurDistance = 0;
    m_offset = FloatSize();
}

// Instead of integer division, we use 17.15 for fixed-point division.
static const int BlurSumShift = 15;

// Check http://www.w3.org/TR/SVG/filters.html#feGaussianBlur.
// As noted in the SVG filter specification, running box blur 3x
// approximates a real gaussian blur nicely.

void ContextShadow::blurLayerImage(unsigned char* imageData, const IntSize& size, int rowStride)
{
    int channels[4] = { 3, 0, 1, 3 };
    int d = std::max(2, static_cast<int>(floorf((2 / 3.f) * m_blurDistance)));
    int dmax = d >> 1;
    int dmin = dmax - 1 + (d & 1);
    if (dmin < 0)
        dmin = 0;

    // Two stages: horizontal and vertical
    for (int k = 0; k < 2; ++k) {

        unsigned char* pixels = imageData;
        int stride = (!k) ? 4 : rowStride;
        int delta = (!k) ? rowStride : 4;
        int jfinal = (!k) ? size.height() : size.width();
        int dim = (!k) ? size.width() : size.height();

        for (int j = 0; j < jfinal; ++j, pixels += delta) {

            // For each step, we blur the alpha in a channel and store the result
            // in another channel for the subsequent step.
            // We use sliding window algorithm to accumulate the alpha values.
            // This is much more efficient than computing the sum of each pixels
            // covered by the box kernel size for each x.

            for (int step = 0; step < 3; ++step) {
                int side1 = (!step) ? dmin : dmax;
                int side2 = (step == 1) ? dmin : dmax;
                int pixelCount = side1 + 1 + side2;
                int invCount = ((1 << BlurSumShift) + pixelCount - 1) / pixelCount;
                int ofs = 1 + side2;
                int alpha1 = pixels[channels[step]];
                int alpha2 = pixels[(dim - 1) * stride + channels[step]];
                unsigned char* ptr = pixels + channels[step + 1];
                unsigned char* prev = pixels + stride + channels[step];
                unsigned char* next = pixels + ofs * stride + channels[step];

                int i;
                int sum = side1 * alpha1 + alpha1;
                int limit = (dim < side2 + 1) ? dim : side2 + 1;
                for (i = 1; i < limit; ++i, prev += stride)
                    sum += *prev;
                if (limit <= side2)
                    sum += (side2 - limit + 1) * alpha2;

                limit = (side1 < dim) ? side1 : dim;
                for (i = 0; i < limit; ptr += stride, next += stride, ++i, ++ofs) {
                    *ptr = (sum * invCount) >> BlurSumShift;
                    sum += ((ofs < dim) ? *next : alpha2) - alpha1;
                }
                prev = pixels + channels[step];
                for (; ofs < dim; ptr += stride, prev += stride, next += stride, ++i, ++ofs) {
                    *ptr = (sum * invCount) >> BlurSumShift;
                    sum += (*next) - (*prev);
                }
                for (; i < dim; ptr += stride, prev += stride, ++i) {
                    *ptr = (sum * invCount) >> BlurSumShift;
                    sum += alpha2 - (*prev);
                }
            }
        }
    }
}

void ContextShadow::calculateLayerBoundingRect(const FloatRect& layerArea, const IntRect& clipRect)
{
    // Calculate the destination of the blurred layer.
    FloatRect destinationRect(layerArea);
    destinationRect.move(m_offset);
    m_layerRect = enclosingIntRect(destinationRect);

    // We expand the area by the blur radius to give extra space for the blur transition.
    m_layerRect.inflate(m_type == BlurShadow ? m_blurDistance : 0);

    if (!clipRect.contains(m_layerRect)) {
        // No need to have the buffer larger than the clip.
        m_layerRect.intersect(clipRect);

        // If we are totally outside the clip region, we aren't painting at all.
        if (m_layerRect.isEmpty())
            return;

        // We adjust again because the pixels at the borders are still
        // potentially affected by the pixels outside the buffer.
        if (m_type == BlurShadow)
            m_layerRect.inflate(m_type == BlurShadow ? m_blurDistance : 0);
    }
}


//////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/ContextShadowQt.cpp

//#include "config.h"
//#include "ContextShadow.h"

#include <QPainter>
#include <QTimerEvent>

// ContextShadow needs a scratch image as the buffer for the blur filter.
// Instead of creating and destroying the buffer for every operation,
// we create a buffer which will be automatically purged via a timer.

class ShadowBuffer: public QObject {
public:
    ShadowBuffer(QObject* parent = 0);

    QImage* scratchImage(const QSize& size);

    void schedulePurge();

protected:
    void timerEvent(QTimerEvent* event);

private:
    QImage image;
    int timerId;
};

ShadowBuffer::ShadowBuffer(QObject* parent)
    : QObject(parent)
    , timerId(0)
{
}

QImage* ShadowBuffer::scratchImage(const QSize& size)
{
    int width = size.width();
    int height = size.height();

    // We do not need to recreate the buffer if the buffer is reasonably
    // larger than the requested size. However, if the requested size is
    // much smaller than our buffer, reduce our buffer so that we will not
    // keep too many allocated pixels for too long.
    if (!image.isNull() && (image.width() > width) && (image.height() > height))
        if (((2 * width) > image.width()) && ((2 * height) > image.height())) {
            image.fill(Qt::transparent);
            return &image;
        }

    // Round to the nearest 32 pixels so we do not grow the buffer everytime
    // there is larger request by 1 pixel.
    width = (1 + (width >> 5)) << 5;
    height = (1 + (height >> 5)) << 5;

    image = QImage(width, height, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    return &image;
}

void ShadowBuffer::schedulePurge()
{
    static const double BufferPurgeDelay = 2; // seconds
    killTimer(timerId);
    timerId = startTimer(BufferPurgeDelay * 1000);
}

void ShadowBuffer::timerEvent(QTimerEvent* event)
{
    if (event->timerId() == timerId) {
        killTimer(timerId);
        image = QImage();
    }
    QObject::timerEvent(event);
}

Q_GLOBAL_STATIC(ShadowBuffer, scratchShadowBuffer)

PlatformContext ContextShadow::beginShadowLayer(PlatformContext p, const FloatRect& layerArea)
{
    QRect clipRect;
    if (p->hasClipping())
#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
        clipRect = p->clipBoundingRect().toAlignedRect();
#else
        clipRect = p->clipRegion().boundingRect();
#endif
    else
        clipRect = p->transform().inverted().mapRect(p->window());

    calculateLayerBoundingRect(layerArea, IntRect(clipRect.x(), clipRect.y(), clipRect.width(), clipRect.height()));

    // Don't paint if we are totally outside the clip region.
    if (m_layerRect.isEmpty())
        return 0;

    ShadowBuffer* shadowBuffer = scratchShadowBuffer();
    QImage* shadowImage = shadowBuffer->scratchImage(m_layerRect.size());
    m_layerImage = QImage(*shadowImage);

    m_layerContext = new QPainter;
    m_layerContext->begin(&m_layerImage);
    m_layerContext->setFont(p->font());
    m_layerContext->translate(m_offset.width(), m_offset.height());

    // The origin is now the top left corner of the scratch image.
    m_layerContext->translate(-m_layerRect.x(), -m_layerRect.y());

    return m_layerContext;
}

void ContextShadow::endShadowLayer(PlatformContext p)
{
    m_layerContext->end();
    delete m_layerContext;
    m_layerContext = 0;

    if (m_type == BlurShadow) {
        blurLayerImage(m_layerImage.bits(), IntSize(m_layerImage.width(), m_layerImage.height()),
                       m_layerImage.bytesPerLine());

        // "Colorize" with the right shadow color.
        QPainter p(&m_layerImage);
        p.setCompositionMode(QPainter::CompositionMode_SourceIn);
        p.fillRect(m_layerImage.rect(), m_color.rgb());
        p.end();
    }

    p->drawImage(m_layerRect.topLeft(), m_layerImage);
    scratchShadowBuffer()->schedulePurge();
}
