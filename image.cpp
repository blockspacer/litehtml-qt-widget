#include "common.h"

#include "image.h"
#include "graphicscontext.h"
#include "intrect.h"
#include "affinetransform.h"
#include "bitmapimage.h"
#include "mimetyperegistry.h"
#include "stillimageqt.h"
#include "sharedbuffer.h"

#include <cmath>

/*#include "config.h"
#include "Image.h"

#include "AffineTransform.h"
#include "BitmapImage.h"
#include "GraphicsContext.h"
#include "IntRect.h"
#include "MIMETypeRegistry.h"
#include "SharedBuffer.h"
#include <math.h>
#include <wtf/StdLibExtras.h>*/

Image::Image(ImageObserver* observer)
    : m_imageObserver(observer)
{
}

Image::~Image()
{
}

Image* Image::nullImage()
{
    //ASSERT(isMainThread());
    DEFINE_STATIC_LOCAL(RefPtr<Image>, nullImage, (BitmapImage::create()));;
    return nullImage.get();
}

bool Image::supportsType(const String& type)
{
    return MIMETypeRegistry::isSupportedImageResourceMIMEType(type);
}

bool Image::setData(PassRefPtr<SharedBuffer> data, bool allDataReceived)
{
    m_data = data;
    if (!m_data.get())
        return true;

    int length = m_data->size();
    if (!length)
        return true;

    return dataChanged(allDataReceived);
}

void Image::fillWithSolidColor(GraphicsContext* ctxt, const FloatRect& dstRect, const Color& color, ColorSpace styleColorSpace, CompositeOperator op)
{
    if (color.alpha() <= 0)
        return;

    ctxt->save();
    ctxt->setCompositeOperation(!color.hasAlpha() && op == CompositeSourceOver ? CompositeCopy : op);
    ctxt->fillRect(dstRect, color, styleColorSpace);
    ctxt->restore();
}

static inline FloatSize calculatePatternScale(const FloatRect& dstRect, const FloatRect& srcRect, Image::TileRule hRule, Image::TileRule vRule)
{
    float scaleX = 1.0f, scaleY = 1.0f;

    if (hRule == Image::StretchTile)
        scaleX = dstRect.width() / srcRect.width();
    if (vRule == Image::StretchTile)
        scaleY = dstRect.height() / srcRect.height();

    if (hRule == Image::RepeatTile)
        scaleX = scaleY;
    if (vRule == Image::RepeatTile)
        scaleY = scaleX;

    return FloatSize(scaleX, scaleY);
}


void Image::drawTiled(GraphicsContext* ctxt, const FloatRect& destRect, const FloatPoint& srcPoint, const FloatSize& scaledTileSize, ColorSpace styleColorSpace, CompositeOperator op)
{
    if (mayFillWithSolidColor()) {
        fillWithSolidColor(ctxt, destRect, solidColor(), styleColorSpace, op);
        return;
    }

    FloatSize intrinsicTileSize = size();
    if (hasRelativeWidth())
        intrinsicTileSize.setWidth(scaledTileSize.width());
    if (hasRelativeHeight())
        intrinsicTileSize.setHeight(scaledTileSize.height());

    FloatSize scale(scaledTileSize.width() / intrinsicTileSize.width(),
                    scaledTileSize.height() / intrinsicTileSize.height());

    FloatRect oneTileRect;
    oneTileRect.setX(destRect.x() + fmodf(fmodf(-srcPoint.x(), scaledTileSize.width()) - scaledTileSize.width(), scaledTileSize.width()));
    oneTileRect.setY(destRect.y() + fmodf(fmodf(-srcPoint.y(), scaledTileSize.height()) - scaledTileSize.height(), scaledTileSize.height()));
    oneTileRect.setSize(scaledTileSize);

    // Check and see if a single draw of the image can cover the entire area we are supposed to tile.
    if (oneTileRect.contains(destRect)) {
        FloatRect visibleSrcRect;
        visibleSrcRect.setX((destRect.x() - oneTileRect.x()) / scale.width());
        visibleSrcRect.setY((destRect.y() - oneTileRect.y()) / scale.height());
        visibleSrcRect.setWidth(destRect.width() / scale.width());
        visibleSrcRect.setHeight(destRect.height() / scale.height());
        draw(ctxt, destRect, visibleSrcRect, styleColorSpace, op);
        return;
    }

    AffineTransform patternTransform = AffineTransform().scaleNonUniform(scale.width(), scale.height());
    FloatRect tileRect(FloatPoint(), intrinsicTileSize);
    drawPattern(ctxt, tileRect, patternTransform, oneTileRect.location(), styleColorSpace, op, destRect);

    startAnimation();
}

// FIXME: Merge with the other drawTiled eventually, since we need a combination of both for some things.
void Image::drawTiled(GraphicsContext* ctxt, const FloatRect& dstRect, const FloatRect& srcRect, TileRule hRule, TileRule vRule, ColorSpace styleColorSpace, CompositeOperator op)
{
    if (mayFillWithSolidColor()) {
        fillWithSolidColor(ctxt, dstRect, solidColor(), styleColorSpace, op);
        return;
    }

    // FIXME: We do not support 'round' yet.  For now just map it to 'repeat'.
    if (hRule == RoundTile)
        hRule = RepeatTile;
    if (vRule == RoundTile)
        vRule = RepeatTile;

    FloatSize scale = calculatePatternScale(dstRect, srcRect, hRule, vRule);
    AffineTransform patternTransform = AffineTransform().scaleNonUniform(scale.width(), scale.height());

    // We want to construct the phase such that the pattern is centered (when stretch is not
    // set for a particular rule).
    float hPhase = scale.width() * srcRect.x();
    float vPhase = scale.height() * srcRect.y();
    if (hRule == Image::RepeatTile)
        hPhase -= fmodf(dstRect.width(), scale.width() * srcRect.width()) / 2.0f;
    if (vRule == Image::RepeatTile)
        vPhase -= fmodf(dstRect.height(), scale.height() * srcRect.height()) / 2.0f;
    FloatPoint patternPhase(dstRect.x() - hPhase, dstRect.y() - vPhase);

    drawPattern(ctxt, srcRect, patternTransform, patternPhase, styleColorSpace, op, dstRect);

    startAnimation();
}

//////////////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/ImageQt.cpp

#include "common.h"

#include "image.h"

#include "affinetransform.h"
#include "imageobserver.h"
#include "bitmapimage.h"
#include "floatrect.h"
//#include "PlatformString.h"
#include "graphicscontext.h"
#include "stillimageqt.h"
//#include "qwebsettings.h"

#include <QPixmap>
#include <QPainter>
#include <QImage>
#include <QImageReader>
#include <QTransform>

#include <QDebug>

#include <math.h>

// This function loads resources into WebKit
/*static QPixmap loadResourcePixmap(const char *name)
{
    QPixmap pixmap;
    if (qstrcmp(name, "missingImage") == 0)
        pixmap = QWebSettings::webGraphic(QWebSettings::MissingImageGraphic);
    else if (qstrcmp(name, "nullPlugin") == 0)
        pixmap = QWebSettings::webGraphic(QWebSettings::MissingPluginGraphic);
    else if (qstrcmp(name, "urlIcon") == 0)
        pixmap = QWebSettings::webGraphic(QWebSettings::DefaultFrameIconGraphic);
    else if (qstrcmp(name, "textAreaResizeCorner") == 0)
        pixmap = QWebSettings::webGraphic(QWebSettings::TextAreaSizeGripCornerGraphic);
    else if (qstrcmp(name, "deleteButton") == 0)
        pixmap = QWebSettings::webGraphic(QWebSettings::DeleteButtonGraphic);
    else if (!qstrcmp(name, "inputSpeech"))
        pixmap = QWebSettings::webGraphic(QWebSettings::InputSpeechButtonGraphic);

    return pixmap;
}*/

bool FrameData::clear(bool clearMetadata)
{
    if (clearMetadata)
        m_haveMetadata = false;

    if (m_frame) {
        delete m_frame;
        m_frame = 0;
        return true;
    }
    return false;
}


// ================================================
// Image Class
// ================================================

/*PassRefPtr<Image> Image::loadPlatformResource(const char* name)
{
    return StillImage::create(loadResourcePixmap(name));
}*/

void Image::drawPattern(GraphicsContext* ctxt, const FloatRect& tileRect, const AffineTransform& patternTransform,
                        const FloatPoint& phase, ColorSpace, CompositeOperator op, const FloatRect& destRect)
{
    QPixmap* framePixmap = nativeImageForCurrentFrame();
    if (!framePixmap) // If it's too early we won't have an image yet.
        return;

    QPixmap pixmap = *framePixmap;
    QRect tr = QRectF(tileRect).toRect();
    if (tr.x() || tr.y() || tr.width() != pixmap.width() || tr.height() != pixmap.height())
        pixmap = pixmap.copy(tr);

    QBrush b(pixmap);
    b.setTransform(patternTransform);
    ctxt->save();
    ctxt->setCompositeOperation(op);
    QPainter* p = ctxt->platformContext();
    if (!pixmap.hasAlpha() && p->compositionMode() == QPainter::CompositionMode_SourceOver)
        p->setCompositionMode(QPainter::CompositionMode_Source);
    p->setBrushOrigin(phase);
    p->fillRect(destRect, b);
    ctxt->restore();

    if (imageObserver())
        imageObserver()->didDraw(this);
}

BitmapImage::BitmapImage(QPixmap* pixmap, ImageObserver* observer)
    : Image(observer)
    , m_currentFrame(0)
    , m_frames(0)

    // TODO
    //, m_frameTimer(0)

    , m_repetitionCount(cAnimationNone)
    , m_repetitionCountStatus(Unknown)
    , m_repetitionsComplete(0)
    , m_isSolidColor(false)
    , m_checkedForSolidColor(false)
    , m_animationFinished(true)
    , m_allDataReceived(true)
    , m_haveSize(true)
    , m_sizeAvailable(true)
    , m_decodedSize(0)
    , m_haveFrameCount(true)
    , m_frameCount(1)
{
    initPlatformData();

    int width = pixmap->width();
    int height = pixmap->height();
    m_decodedSize = width * height * 4;
    m_size = IntSize(width, height);

    //m_frames.grow(1);
    m_frames.reserve(1);

    m_frames[0].m_frame = pixmap;
    m_frames[0].m_hasAlpha = pixmap->hasAlpha();
    m_frames[0].m_haveMetadata = true;
    checkForSolidColor();
}

void BitmapImage::initPlatformData()
{
}

void BitmapImage::invalidatePlatformData()
{
}

// Drawing Routines
void BitmapImage::draw(GraphicsContext* ctxt, const FloatRect& dst,
                       const FloatRect& src, ColorSpace styleColorSpace, CompositeOperator op)
{
    FloatRect normalizedDst = dst.normalized();
    FloatRect normalizedSrc = src.normalized();

    startAnimation();

    QPixmap* image = nativeImageForCurrentFrame();
    if (!image)
        return;

    if (mayFillWithSolidColor()) {
        fillWithSolidColor(ctxt, normalizedDst, solidColor(), styleColorSpace, op);
        return;
    }

    QPainter* painter(ctxt->platformContext());

    QPainter::CompositionMode compositionMode = GraphicsContext::toQtCompositionMode(op);

    if (!image->hasAlpha() && painter->compositionMode() == QPainter::CompositionMode_SourceOver)
        compositionMode = QPainter::CompositionMode_Source;

    QPainter::CompositionMode lastCompositionMode = painter->compositionMode();
    painter->setCompositionMode(compositionMode);

    FloatSize shadowOffset;
    float shadowBlur;
    Color shadowColor;
    if (ctxt->getShadow(shadowOffset, shadowBlur, shadowColor)) {
        FloatRect shadowImageRect(normalizedDst);
        shadowImageRect.move(shadowOffset.width(), shadowOffset.height());

        QImage shadowImage(QSize(static_cast<int>(normalizedSrc.width()), static_cast<int>(normalizedSrc.height())), QImage::Format_ARGB32_Premultiplied);
        QPainter p(&shadowImage);
        p.setCompositionMode(QPainter::CompositionMode_Source);
        p.fillRect(shadowImage.rect(), shadowColor);
        p.setCompositionMode(QPainter::CompositionMode_DestinationIn);
        p.drawPixmap(QRect(0, 0, normalizedDst.width(), normalizedDst.height()), *image, normalizedSrc);
        p.end();
        painter->drawImage(shadowImageRect, shadowImage, normalizedSrc);
    }

    // Test using example site at
    // http://www.meyerweb.com/eric/css/edge/complexspiral/demo.html
    painter->drawPixmap(normalizedDst, *image, normalizedSrc);

    painter->setCompositionMode(lastCompositionMode);

    if (imageObserver())
        imageObserver()->didDraw(this);
}

void BitmapImage::checkForSolidColor()
{
    m_isSolidColor = false;
    m_checkedForSolidColor = true;

    if (frameCount() > 1)
        return;

    QPixmap* framePixmap = frameAtIndex(0);
    if (!framePixmap || framePixmap->width() != 1 || framePixmap->height() != 1)
        return;

    m_isSolidColor = true;
    m_solidColor = QColor::fromRgba(framePixmap->toImage().pixel(0, 0));
}
