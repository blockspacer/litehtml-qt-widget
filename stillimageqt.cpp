#include "stillimageqt.h"

#include "common.h"

#include "stillimageqt.h"

#include "graphicscontext.h"
#include "intsize.h"

#include <QPainter>

StillImage::StillImage(const QPixmap& pixmap)
    : m_pixmap(new QPixmap(pixmap))
    , m_ownsPixmap(true)
{}

StillImage::StillImage(const QPixmap* pixmap)
    : m_pixmap(pixmap)
    , m_ownsPixmap(false)
{}

StillImage::~StillImage()
{
    if (m_ownsPixmap)
        delete m_pixmap;
}

IntSize StillImage::size() const
{
    return IntSize(m_pixmap->width(), m_pixmap->height());
}

NativeImagePtr StillImage::nativeImageForCurrentFrame()
{
    return const_cast<NativeImagePtr>(m_pixmap);
}

void StillImage::draw(GraphicsContext* ctxt, const FloatRect& dst,
                      const FloatRect& src, ColorSpace, CompositeOperator op)
{
    if (m_pixmap->isNull())
        return;


    FloatRect normalizedSrc = src.normalized();
    FloatRect normalizedDst = dst.normalized();

    QPainter* painter = ctxt->platformContext();
    QPainter::CompositionMode oldCompositionMode = painter->compositionMode();

    ctxt->setCompositeOperation(op);

    FloatSize shadowOffset;
    float shadowBlur;
    Color shadowColor;

    // TODO

    /*if (ctxt->getShadow(shadowOffset, shadowBlur, shadowColor)) {
        FloatRect shadowImageRect(normalizedDst);
        shadowImageRect.move(shadowOffset.width(), shadowOffset.height());

        QImage shadowImage(QSize(static_cast<int>(normalizedSrc.width()), static_cast<int>(normalizedSrc.height())), QImage::Format_ARGB32_Premultiplied);
        QPainter p(&shadowImage);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(shadowImage.rect(), shadowColor);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRect(0, 0, normalizedDst.width(), normalizedDst.height()), *m_pixmap, normalizedSrc);
        p.end();
        painter->drawImage(shadowImageRect, shadowImage, normalizedSrc);
    }*/

    painter->drawPixmap(normalizedDst, *m_pixmap, normalizedSrc);
    painter->setCompositionMode(oldCompositionMode);
}
