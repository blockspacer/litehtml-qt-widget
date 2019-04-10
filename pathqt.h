#ifndef PATHQT_H
#define PATHQT_H

//#include "types.h"
#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>

#include "fontcache.h"
#include <cmath>

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/PathQt.cpp#L266
//#define DEGREES(t) ((t) * 180.0 / M_PI)

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/FloatPointQt.cpp
#define FloatPoint QPointF

//https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/FloatRectQt.cpp
#define FloatRect QRectF

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/FloatSize.h
#define FloatSize QRectF

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/JavaScriptCore/wtf/MathExtras.h
#ifndef M_PI
const double piDouble = 3.14159265358979323846;
const float piFloat = 3.14159265358979323846f;
#else
const double piDouble = M_PI;
const float piFloat = static_cast<float>(M_PI);
#endif

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/Path.h#L44
#include <qpainterpath.h>
typedef QPainterPath PlatformPath;
typedef PlatformPath PlatformPathPtr;

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/transforms/AffineTransform.h#L169
typedef QTransform AffineTransform;

enum WindRule {
    RULE_NONZERO = 0,
    RULE_EVENODD = 1
};

enum PathElementType {
    PathElementMoveToPoint,
    PathElementAddLineToPoint,
    PathElementAddQuadCurveToPoint,
    PathElementAddCurveToPoint,
    PathElementCloseSubpath
};

struct PathElement {
    PathElementType type;
    FloatPoint* points;
};

typedef void (*PathApplierFunction)(void* info, const PathElement*);

class Path
{
    public:
        Path();
        ~Path();

        Path(const Path&);
        Path& operator=(const Path&);

        void swap(Path& other) { std::swap(m_path, other.m_path); }

        bool contains(const FloatPoint&, WindRule rule = RULE_NONZERO) const;
        //bool strokeContains(StrokeStyleApplier*, const FloatPoint&) const;
        FloatRect boundingRect() const;
        //FloatRect strokeBoundingRect(StrokeStyleApplier* = 0);

        float length();
        FloatPoint pointAtLength(float length, bool& ok);
        float normalAngleAtLength(float length, bool& ok);

        void clear();
        bool isEmpty() const;
        // Gets the current point of the current path, which is conceptually the final point reached by the path so far.
        // Note the Path can be empty (isEmpty() == true) and still have a current point.
        bool hasCurrentPoint() const;
        FloatPoint currentPoint() const;

        void moveTo(const FloatPoint&);
        void addLineTo(const FloatPoint&);
        void addQuadCurveTo(const FloatPoint& controlPoint, const FloatPoint& endPoint);
        void addBezierCurveTo(const FloatPoint& controlPoint1, const FloatPoint& controlPoint2, const FloatPoint& endPoint);
        void addArcTo(const FloatPoint&, const FloatPoint&, float radius);
        void closeSubpath();

        void addArc(const FloatPoint&, float radius, float startAngle, float endAngle, bool anticlockwise);
        void addRect(const FloatRect&);
        void addEllipse(const FloatRect&);
        void addRoundedRect(const FloatRect&, const FloatSize& roundingRadii);
        void addRoundedRect(const FloatRect&, const FloatSize& topLeftRadius, const FloatSize& topRightRadius, const FloatSize& bottomLeftRadius, const FloatSize& bottomRightRadius);

        void translate(const FloatSize&);

        PlatformPathPtr platformPath() const { return m_path; }

        void apply(void* info, PathApplierFunction) const;
        void transform(const AffineTransform&);

    private:
        PlatformPathPtr m_path;
};

#endif // PATHQT_H
