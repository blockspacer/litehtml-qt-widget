#pragma once

#include "common.h"

#include "color.h"
#include "floatrect.h"
//#include "graphicscontext.h"
#include "intrect.h"
//#include "RefCounted.h"

/*#include "Color.h"
#include "FloatRect.h"
#include "GraphicsContext.h"
#include "IntRect.h"
#include "RefCounted.h"*/

#include <QImage>
class QPainter;
typedef QImage PlatformImage;
typedef QPainter* PlatformContext;

// This is to track and keep the shadow state. We use this rather than
// using GraphicsContextState to allow possible optimizations (right now
// only to determine the shadow type, but in future it might covers things
// like cached scratch image, persistent shader, etc).

// This class should be copyable since GraphicsContextQt keeps a stack of
// the shadow state for savePlatformState and restorePlatformState.

class ContextShadow {
public:
    enum {
        NoShadow,
        SolidShadow,
        BlurShadow
    } m_type;

    Color m_color;
    int m_blurDistance;
    FloatSize m_offset;

    ContextShadow();
    ContextShadow(const Color&, float radius, const FloatSize& offset);

    void clear();

    // The pair beginShadowLayer and endShadowLayer creates a temporary image
    // where the caller can draw onto, using the returned context. This context
    // must be used only to draw between the call to beginShadowLayer and
    // endShadowLayer.
    //
    // Note: multiple/nested shadow layers are NOT allowed.
    //
    // The current clip region will be used to optimize the size of the
    // temporary image. Thus, the original context should not change any
    // clipping until endShadowLayer. If the shadow will be completely outside
    // the clipping region, beginShadowLayer will return 0.
    //
    // The returned context will have the transformation matrix and clipping
    // properly initialized to start doing the painting (no need to account for
    // the shadow offset), however it will not have the same render hints, pen,
    // brush, etc as the passed context. This is intentional, usually shadows
    // have different properties than the shapes which cast them.
    //
    // Once endShadowLayer is called, the temporary image will be drawn with the
    // original context. If blur radius is specified, the shadow will be
    // filtered first.

    PlatformContext beginShadowLayer(PlatformContext, const FloatRect& layerArea);
    void endShadowLayer(PlatformContext);
    static void purgeScratchBuffer();
/*#if PLATFORM(CAIRO)
    void drawRectShadow(GraphicsContext* context, const IntRect& rect, const IntSize& topLeftRadius, const IntSize& topRightRadius, const IntSize& bottomLeftRadius, const IntSize& bottomRightRadius);
#endif*/

//#if PLATFORM(QT)
    QPointF offset() { return QPointF(m_offset.width(), m_offset.height()); }
//#endif


private:
    IntRect m_layerRect;
    PlatformImage m_layerImage;
    PlatformContext m_layerContext;

    void blurLayerImage(unsigned char*, const IntSize& imageSize, int stride);
    void calculateLayerBoundingRect(const FloatRect& layerArea, const IntRect& clipRect);
/*#if PLATFORM(CAIRO)
    void drawRectShadowWithoutTiling(PlatformContext context, const IntRect& shadowRect, const IntSize& topLeftRadius, const IntSize& topRightRadius, const IntSize& bottomLeftRadius, const IntSize& bottomRightRadius, float alpha);
#endif*/
};
