#pragma once

#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>

#include "fontcache.h"
#include <cmath>
#include <memory>

#include "pathqt.h"

#include "color.h"

#include "common.h"

#include "gradient.h"

#include "image.h"
#include "stillimageqt.h"

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/GraphicsContext.h
//class GraphicsContextPrivate;
//class GraphicsContextPlatformPrivate;

    const int cMisspellingLineThickness = 3;
    const int cMisspellingLinePatternWidth = 4;
    const int cMisspellingLinePatternGapWidth = 1;

    // These bits can be ORed together for a total of 8 possible text drawing modes.
    const int cTextInvisible = 0;
    const int cTextFill = 1;
    const int cTextStroke = 2;
    const int cTextClip = 4;

    enum StrokeStyle {
        NoStroke,
        SolidStroke,
        DottedStroke,
        DashedStroke
    };

    enum InterpolationQuality {
        InterpolationDefault,
        InterpolationNone,
        InterpolationLow,
        InterpolationMedium,
        InterpolationHigh
    };

/////////////////////////


class GraphicsContextPlatformPrivate /*: public Noncopyable*/ {
public:
    GraphicsContextPlatformPrivate(QPainter* painter);
    ~GraphicsContextPlatformPrivate();

    inline QPainter* p() const
    {
      return painter;
        /*if (layers.isEmpty())
            return painter;
        return &layers.top()->painter;*/
    }

    bool antiAliasingForRectsAndLines;

    //QStack<TransparencyLayer*> layers;
    // Counting real layers. Required by inTransparencyLayer() calls
    // For example, layers with valid alphaMask are not real layers
    int layerCount;

    // reuse this brush for solid color (to prevent expensive QBrush construction)
    QBrush solidColor;

    InterpolationQuality imageInterpolationQuality;

    // Only used by SVG for now.
    QPainterPath currentPath;

    //ContextShadow shadow;
    //QStack<ContextShadow> shadowStack;

    /*bool hasShadow() const
    {
        return shadow.m_type != ContextShadow::NoShadow;
    }*/

    inline void clearCurrentPath()
    {
        if (!currentPath.elementCount())
            return;
        currentPath = QPainterPath();
    }

    QRectF clipBoundingRect() const
    {
#if QT_VERSION >= QT_VERSION_CHECK(4, 8, 0)
        return p()->clipBoundingRect();
#else
        return p()->clipRegion().boundingRect();
#endif
    }

private:
    QPainter* painter;
};


/////////////////////////



    struct GraphicsContextState {
        GraphicsContextState()
            : textDrawingMode(cTextFill)
            , strokeStyle(SolidStroke)
            , strokeThickness(0)
            , strokeColor(Color::black)
            , strokeColorSpace(DeviceColorSpace)
            , fillRule(RULE_NONZERO)
            , fillColor(Color::black)
            , fillColorSpace(DeviceColorSpace)
            , shouldAntialias(true)
            , paintingDisabled(false)
            , shadowBlur(0)
            , shadowsIgnoreTransforms(false)
        {
        }

        int textDrawingMode;

        StrokeStyle strokeStyle;
        float strokeThickness;
        Color strokeColor;
        ColorSpace strokeColorSpace;
        RefPtr<Gradient> strokeGradient;
        /*RefPtr<Pattern> strokePattern;*/

        WindRule fillRule;
        Color fillColor;
        ColorSpace fillColorSpace;
        RefPtr<Gradient> fillGradient;
        /*RefPtr<Pattern> fillPattern;*/

        bool shouldAntialias;

        bool paintingDisabled;

        FloatSize shadowOffset;
        float shadowBlur;
        Color shadowColor;

        bool shadowsIgnoreTransforms;

        TransformationMatrix pathTransform;

    };

    class GraphicsContextPrivate /*: public Noncopyable*/ {
    public:
        GraphicsContextPrivate()
            : m_updatingControlTints(false)
        {
        }

        GraphicsContextState state;
        QVector<GraphicsContextState> stack;
        bool m_updatingControlTints;

    };


/////////////////////////

class GraphicsContext
{
public:
        GraphicsContext(PlatformGraphicsContext*);
        ~GraphicsContext();

        PlatformGraphicsContext* platformContext() const;

        float strokeThickness() const;
        void setStrokeThickness(float);
        StrokeStyle strokeStyle() const;
        void setStrokeStyle(const StrokeStyle& style);
        Color strokeColor() const;
        ColorSpace strokeColorSpace() const;
        void setStrokeColor(const Color&, ColorSpace);

        /*void setStrokePattern(PassRefPtr<Pattern>);
        Pattern* strokePattern() const;*/

        void setStrokeGradient(PassRefPtr<Gradient>);
        Gradient* strokeGradient() const;

        WindRule fillRule() const;
        void setFillRule(WindRule);
        Color fillColor() const;
        ColorSpace fillColorSpace() const;
        void setFillColor(const Color&, ColorSpace);

        /*void setFillPattern(PassRefPtr<Pattern>);
        Pattern* fillPattern() const;*/

        void setFillGradient(PassRefPtr<Gradient>);
        Gradient* fillGradient() const;

        void setShadowsIgnoreTransforms(bool);

        void setShouldAntialias(bool);
        bool shouldAntialias() const;

        void save();
        void restore();

        // These draw methods will do both stroking and filling.
        // FIXME: ...except drawRect(), which fills properly but always strokes
        // using a 1-pixel stroke inset from the rect borders (of the correct
        // stroke color).
        void drawRect(const IntRect&);
        void drawLine(const IntPoint&, const IntPoint&);
        void drawEllipse(const IntRect&);
        void drawConvexPolygon(size_t numPoints, const FloatPoint*, bool shouldAntialias = false);

        void drawPath();
        void fillPath();
        void strokePath();

        // Arc drawing (used by border-radius in CSS) just supports stroking at the moment.
        void strokeArc(const IntRect&, int startAngle, int angleSpan);

        void fillRect(const FloatRect&);
        void fillRect(const FloatRect&, const Color&, ColorSpace);


        void fillRect(const FloatRect&, Generator&);


        void fillRoundedRect(const IntRect&, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight, const Color&, ColorSpace);

        void clearRect(const FloatRect&);

        void strokeRect(const FloatRect&);
        void strokeRect(const FloatRect&, float lineWidth);


        void drawImage(Image*, ColorSpace styleColorSpace, const IntPoint&, CompositeOperator = CompositeSourceOver);
        void drawImage(Image*, ColorSpace styleColorSpace, const IntRect&, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImage(Image*, ColorSpace styleColorSpace, const IntPoint& destPoint, const IntRect& srcRect, CompositeOperator = CompositeSourceOver);
        void drawImage(Image*, ColorSpace styleColorSpace, const IntRect& destRect, const IntRect& srcRect, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImage(Image*, ColorSpace styleColorSpace, const FloatRect& destRect, const FloatRect& srcRect = FloatRect(0, 0, -1, -1),
                       CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawTiledImage(Image*, ColorSpace styleColorSpace, const IntRect& destRect, const IntPoint& srcPoint, const IntSize& tileSize,
                       CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawTiledImage(Image*, ColorSpace styleColorSpace, const IntRect& destRect, const IntRect& srcRect,
                            Image::TileRule hRule = Image::StretchTile, Image::TileRule vRule = Image::StretchTile,
                            CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);

        /*void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntPoint&, CompositeOperator = CompositeSourceOver);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntRect&, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntPoint& destPoint, const IntRect& srcRect, CompositeOperator = CompositeSourceOver);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const IntRect& destRect, const IntRect& srcRect, CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        void drawImageBuffer(ImageBuffer*, ColorSpace styleColorSpace, const FloatRect& destRect, const FloatRect& srcRect = FloatRect(0, 0, -1, -1),
                             CompositeOperator = CompositeSourceOver, bool useLowQualityScale = false);
        */

        void setImageInterpolationQuality(InterpolationQuality);
        InterpolationQuality imageInterpolationQuality() const;

        void clip(const FloatRect&);
        void addRoundedRectClip(const IntRect&, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight);
        void addInnerRoundedRectClip(const IntRect&, int thickness);
        void clipOut(const IntRect&);
        void clipOutEllipseInRect(const IntRect&);
        void clipOutRoundedRect(const IntRect&, const IntSize& topLeft, const IntSize& topRight, const IntSize& bottomLeft, const IntSize& bottomRight);
        void clipPath(WindRule);
        void clipConvexPolygon(size_t numPoints, const FloatPoint*, bool antialias = true);
        /*void clipToImageBuffer(ImageBuffer*, const FloatRect&);

        int textDrawingMode();
        void setTextDrawingMode(int);
        */

        /*void drawText(const Font&, const TextRun&, const IntPoint&, int from = 0, int to = -1);
        void drawBidiText(const Font&, const TextRun&, const FloatPoint&);
        void drawHighlightForText(const Font&, const TextRun&, const IntPoint&, int h, const Color& backgroundColor, ColorSpace, int from = 0, int to = -1);
        */

        FloatRect roundToDevicePixels(const FloatRect&);

        /*void drawLineForText(const IntPoint&, int width, bool printing);
        enum TextCheckingLineStyle {
            TextCheckingSpellingLineStyle,
            TextCheckingGrammarLineStyle,
            TextCheckingReplacementLineStyle
        };
        void drawLineForTextChecking(const IntPoint&, int width, TextCheckingLineStyle);*/

        bool paintingDisabled() const;
        void setPaintingDisabled(bool);

        bool updatingControlTints() const;
        void setUpdatingControlTints(bool);

        /*void beginTransparencyLayer(float opacity);
        void endTransparencyLayer();*/

        /*void setShadow(const FloatSize&, float blur, const Color&, ColorSpace);
        bool getShadow(FloatSize&, float&, Color&) const;
        void clearShadow();*/

        void drawFocusRing(const QVector<IntRect>&, int width, int offset, const Color&);
        void drawFocusRing(const QVector<Path>&, int width, int offset, const Color&);

        void setLineCap(LineCap);
        void setLineDash(const DashArray&, float dashOffset);
        void setLineJoin(LineJoin);

        void setMiterLimit(float);

        void setAlpha(float);

        void setCompositeOperation(CompositeOperator);

        void beginPath();
        void addPath(const Path&);

        void clip(const Path&);

        // This clip function is used only by <canvas> code. It allows
        // implementations to handle clipping on the canvas differently since
        // the disipline is different.
        void canvasClip(const Path&);
        void clipOut(const Path&);

        void scale(const FloatSize&);
        void rotate(float angleInRadians);
        void translate(const FloatSize& size) { translate(size.width(), size.height()); }
        void translate(float x, float y);
        IntPoint origin();

        /*void setURLForRect(const KURL&, const IntRect&);*/

        void concatCTM(const AffineTransform&);
        AffineTransform getCTM() const;

        //bool inTransparencyLayer() const;
        PlatformPath* currentPath();
        //void pushTransparencyLayerInternal(const QRect &rect, qreal opacity, QPixmap& alphaMask);
        QPen pen();

        static QPainter::CompositionMode toQtCompositionMode(CompositeOperator op);

        /*ContextShadow* contextShadow();*/

        /*void setSharedGraphicsContext3D(SharedGraphicsContext3D*, DrawingBuffer*, const IntSize&);
        */

        void syncSoftwareCanvas();
        void markDirtyRect(const IntRect&); // Hints that a portion of the backing store is dirty.

    private:
        void savePlatformState();
        void restorePlatformState();

        void setPlatformTextDrawingMode(int);
        //void setPlatformFont(const Font& font);

        void setPlatformStrokeColor(const Color&, ColorSpace);
        void setPlatformStrokeStyle(const StrokeStyle&);
        void setPlatformStrokeThickness(float);
        void setPlatformStrokeGradient(Gradient*);
        /*void setPlatformStrokePattern(Pattern*);*/

        void setPlatformFillColor(const Color&, ColorSpace);
        void setPlatformFillGradient(Gradient*);
        /*void setPlatformFillPattern(Pattern*);*/

        void setPlatformShouldAntialias(bool b);

        /*void setPlatformShadow(const FloatSize&, float blur, const Color&, ColorSpace);

        void clearPlatformShadow();*/

        static void adjustLineToPixelBoundaries(FloatPoint& p1, FloatPoint& p2, float strokeWidth, const StrokeStyle&);

        static GraphicsContextPrivate* createGraphicsContextPrivate();
        static void destroyGraphicsContextPrivate(GraphicsContextPrivate*);

        GraphicsContextPrivate* m_common;
        GraphicsContextPlatformPrivate* m_data; // Deprecated; m_commmon can just be downcasted. To be removed.
    };


