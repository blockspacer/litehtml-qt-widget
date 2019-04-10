#pragma once


#include <qglobal.h>

#include <QtCore>
#include <QVector>

#include <QDebug>
#include <QFont>
#include <QFontMetrics>
#include <QPainter>
#include <QMouseEvent>
#include <QDesktopServices>
#include <QColor>
#include <QPoint>
#include <QPointF>

//#include "types.h"
#include <QPainter>

#include "fontcache.h"

#define    _USE_MATH_DEFINES
#include <iostream>
#include <cmath>
#include <string>

/*#include "floatpoint.h"
#include "floatrect.h"
#include "intpoint.h"
#include "intsize.h"
#include "floatsize.h"
#include "intrect.h"*/

///////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/JavaScriptCore/wtf/FastMalloc.cpp
//#define fastMalloc malloc

//#define tryFastMalloc(string) mallocc<false>(string)

///////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/style/RenderStyleConstants.h#L90

// These have been defined in the order of their precedence for border-collapsing. Do
// not change this order!
enum EBorderStyle { BNONE, BHIDDEN, INSET, GROOVE, RIDGE, OUTSET, DOTTED, DASHED, SOLID, DOUBLE };

///////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/DashArray.h
typedef QVector<float> DashArray;

/////////

#define ASSERT(string) Q_ASSERT(string)

#define CRASH(string) Q_ASSERT(false)

#define ALWAYS_INLINE inline

#define ASSERT_DISABLED 1

////////////

//https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/JavaScriptCore/wtf/NotFound.h
const size_t notFound = static_cast<size_t>(-1);

////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/JavaScriptCore/wtf/StdLibExtras.h
//#define reinterpret_cast_ptr reinterpret_cast

//#define WTF std

////////////

// https://github.com/trevorlinton/webkit.js/blob/15306b5c540ccafc3be008225eafebd1932d356d/deps/WebKit/Source/WTF/wtf/ASCIICType.h#L56
template<typename CharType> inline bool isASCIIDigit(CharType c)
{
    return c >= '0' && c <= '9';
}

template<typename CharType> inline bool isASCIIHexDigit(CharType c)
{
    return isASCIIDigit(c) || ((c | 0x20) >= 'a' && (c | 0x20) <= 'f');
}

template<typename CharType> inline int toASCIIHexValue(CharType c)
{
    //ASSERT(isASCIIHexDigit(c));
    ASSERT(isASCIIHexDigit(c));
    return c < 'A' ? c - '0' : (c - 'A' + 10) & 0xF;
}

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/make-hash-tools.pl#L57
struct NamedColor {
    const char* name;
    int RGBValue;
};

typedef QString String;
typedef char UChar;


//////////////////
// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/FloatConversion.h#L38

    template<typename T>
    float narrowPrecisionToFloat(T);

    template<>
    inline float narrowPrecisionToFloat(double number)
    {
        return static_cast<float>(number);
    }

/////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/PathQt.cpp#L266
//#define DEGREES(t) ((t) * 180.0 / M_PI)

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/FloatPointQt.cpp
//#define FloatPoint QPointF

//https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/FloatRectQt.cpp
//#define FloatRect QRectF

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/FloatSize.h
//#define FloatSize QSizeF

//#define FloatQuad QPolygonF

/////////////////



    typedef QPainter PlatformGraphicsContext;

    //typedef QColor Color;
    //typedef QFont Font;
    //typedef QColorSpace ColorSpace;
    //typedef QPattern Pattern;
    //typedef QPattern Gradient;
    //typedef QRect IntRect;
    //typedef QSize IntSize;
    //typedef QPoint IntPoint;
    //typedef QImage Image;

    //typedef QTransform TransformationMatrix;



    //#define Vector QVector

    #define PassRefPtr std::shared_ptr
    #define RefPtr std::shared_ptr

    #define adoptRef std::make_shared

///////////////////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/ColorSpace.h
enum ColorSpace { DeviceColorSpace, sRGBColorSpace };

///////////////////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/rendering/RenderObject.h#L81

// Sides used when drawing borders and outlines.  This is in RenderObject rather than RenderBoxModelObject since outlines can
// be drawn by SVG around bounding boxes.
enum BoxSide {
    BSTop,
    BSBottom,
    BSLeft,
    BSRight
};

///////////////////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/JavaScriptCore/wtf/MathExtras.h

#include <cmath>
#include <float.h>
#include <stdlib.h>

/*#if OS(SOLARIS)
#include <ieeefp.h>
#endif

#if OS(OPENBSD)
#include <sys/types.h>
#include <machine/ieee.h>
#endif

#if COMPILER(MSVC)
#if OS(WINCE)
#include <stdlib.h>
#endif
#include <limits>
#endif*/

#ifndef M_PI
const double piDouble = 3.14159265358979323846;
const float piFloat = 3.14159265358979323846f;
#else
const double piDouble = M_PI;
const float piFloat = static_cast<float>(M_PI);
#endif

#ifndef M_PI_2
const double piOverTwoDouble = 1.57079632679489661923;
const float piOverTwoFloat = 1.57079632679489661923f;
#else
const double piOverTwoDouble = M_PI_2;
const float piOverTwoFloat = static_cast<float>(M_PI_2);
#endif

#ifndef M_PI_4
const double piOverFourDouble = 0.785398163397448309616;
const float piOverFourFloat = 0.785398163397448309616f;
#else
const double piOverFourDouble = M_PI_4;
const float piOverFourFloat = static_cast<float>(M_PI_4);
#endif

/*#if OS(DARWIN)

// Work around a bug in the Mac OS X libc where ceil(-0.1) return +0.
inline double wtf_ceil(double x) { return copysign(ceil(x), x); }

#define ceil(x) wtf_ceil(x)

#endif

#if OS(SOLARIS)

#ifndef isfinite
inline bool isfinite(double x) { return finite(x) && !isnand(x); }
#endif
#ifndef isinf
inline bool isinf(double x) { return !finite(x) && !isnand(x); }
#endif
#ifndef signbit
inline bool signbit(double x) { return x < 0.0; } // FIXME: Wrong for negative 0.
#endif

#endif

#if OS(OPENBSD)

#ifndef isfinite
inline bool isfinite(double x) { return finite(x); }
#endif
#ifndef signbit
inline bool signbit(double x) { struct ieee_double *p = (struct ieee_double *)&x; return p->dbl_sign; }
#endif

#endif

#if COMPILER(MSVC) || COMPILER(RVCT)

// We must not do 'num + 0.5' or 'num - 0.5' because they can cause precision loss.
static double round(double num)
{
    double integer = ceil(num);
    if (num > 0)
        return integer - num > 0.5 ? integer - 1.0 : integer;
    return integer - num >= 0.5 ? integer - 1.0 : integer;
}
static float roundf(float num)
{
    float integer = ceilf(num);
    if (num > 0)
        return integer - num > 0.5f ? integer - 1.0f : integer;
    return integer - num >= 0.5f ? integer - 1.0f : integer;
}
inline long long llround(double num) { return static_cast<long long>(round(num)); }
inline long long llroundf(float num) { return static_cast<long long>(roundf(num)); }
inline long lround(double num) { return static_cast<long>(round(num)); }
inline long lroundf(float num) { return static_cast<long>(roundf(num)); }
inline double trunc(double num) { return num > 0 ? floor(num) : ceil(num); }

#endif

#if COMPILER(MSVC)
// The 64bit version of abs() is already defined in stdlib.h which comes with VC10
#if COMPILER(MSVC9_OR_LOWER)
inline long long abs(long long num) { return _abs64(num); }
#endif

inline bool isinf(double num) { return !_finite(num) && !_isnan(num); }
inline bool isnan(double num) { return !!_isnan(num); }
inline bool signbit(double num) { return _copysign(1.0, num) < 0; }

inline double nextafter(double x, double y) { return _nextafter(x, y); }
inline float nextafterf(float x, float y) { return x > y ? x - FLT_EPSILON : x + FLT_EPSILON; }

inline double copysign(double x, double y) { return _copysign(x, y); }
inline int isfinite(double x) { return _finite(x); }

// Work around a bug in Win, where atan2(+-infinity, +-infinity) yields NaN instead of specific values.
inline double wtf_atan2(double x, double y)
{
    double posInf = std::numeric_limits<double>::infinity();
    double negInf = -std::numeric_limits<double>::infinity();
    double nan = std::numeric_limits<double>::quiet_NaN();

    double result = nan;

    if (x == posInf && y == posInf)
        result = piOverFourDouble;
    else if (x == posInf && y == negInf)
        result = 3 * piOverFourDouble;
    else if (x == negInf && y == posInf)
        result = -piOverFourDouble;
    else if (x == negInf && y == negInf)
        result = -3 * piOverFourDouble;
    else
        result = ::atan2(x, y);

    return result;
}

// Work around a bug in the Microsoft CRT, where fmod(x, +-infinity) yields NaN instead of x.
inline double wtf_fmod(double x, double y) { return (!isinf(x) && isinf(y)) ? x : fmod(x, y); }

// Work around a bug in the Microsoft CRT, where pow(NaN, 0) yields NaN instead of 1.
inline double wtf_pow(double x, double y) { return y == 0 ? 1 : pow(x, y); }

#define atan2(x, y) wtf_atan2(x, y)
#define fmod(x, y) wtf_fmod(x, y)
#define pow(x, y) wtf_pow(x, y)

#endif // COMPILER(MSVC)*/

inline double deg2rad(double d)  { return d * piDouble / 180.0; }
inline double rad2deg(double r)  { return r * 180.0 / piDouble; }
inline double deg2grad(double d) { return d * 400.0 / 360.0; }
inline double grad2deg(double g) { return g * 360.0 / 400.0; }
inline double turn2deg(double t) { return t * 360.0; }
inline double deg2turn(double d) { return d / 360.0; }
inline double rad2grad(double r) { return r * 200.0 / piDouble; }
inline double grad2rad(double g) { return g * piDouble / 200.0; }

inline float deg2rad(float d)  { return d * piFloat / 180.0f; }
inline float rad2deg(float r)  { return r * 180.0f / piFloat; }
inline float deg2grad(float d) { return d * 400.0f / 360.0f; }
inline float grad2deg(float g) { return g * 360.0f / 400.0f; }
inline float turn2deg(float t) { return t * 360.0f; }
inline float deg2turn(float d) { return d / 360.0f; }
inline float rad2grad(float r) { return r * 200.0f / piFloat; }
inline float grad2rad(float g) { return g * piFloat / 200.0f; }

/*#if !COMPILER(MSVC) && !COMPILER(WINSCW) && !(COMPILER(RVCT) && (OS(SYMBIAN) || PLATFORM(BREWMP)))
using std::isfinite;
using std::isinf;
using std::isnan;
using std::signbit;
#endif*/

/////////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/GraphicsTypes.h

   // Note: These constants exactly match the NSCompositeOperator constants of
    // AppKit on Mac OS X Tiger. If these ever change, we'll need to change the
    // Mac OS X Tiger platform code to map one to the other.
    enum CompositeOperator {
        CompositeClear,
        CompositeCopy,
        CompositeSourceOver,
        CompositeSourceIn,
        CompositeSourceOut,
        CompositeSourceAtop,
        CompositeDestinationOver,
        CompositeDestinationIn,
        CompositeDestinationOut,
        CompositeDestinationAtop,
        CompositeXOR,
        CompositePlusDarker,
        CompositeHighlight,
        CompositePlusLighter
    };

    // FIXME: Currently these constants have to match the values used in the SVG
    // DOM API. That's a mistake. We need to make cut that dependency.
    enum GradientSpreadMethod {
        SpreadMethodPad = 1,
        SpreadMethodReflect = 2,
        SpreadMethodRepeat = 3
    };

    enum LineCap { ButtCap, RoundCap, SquareCap };

    enum LineJoin { MiterJoin, RoundJoin, BevelJoin };

    enum HorizontalAlignment { AlignLeft, AlignRight, AlignHCenter };

    enum TextBaseline { AlphabeticTextBaseline, TopTextBaseline, MiddleTextBaseline, BottomTextBaseline, IdeographicTextBaseline, HangingTextBaseline };

    enum TextAlign { StartTextAlign, EndTextAlign, LeftTextAlign, CenterTextAlign, RightTextAlign };

    String compositeOperatorName(CompositeOperator);
    bool parseCompositeOperator(const String&, CompositeOperator&);

    String lineCapName(LineCap);
    bool parseLineCap(const String&, LineCap&);

    String lineJoinName(LineJoin);
    bool parseLineJoin(const String&, LineJoin&);

    String textAlignName(TextAlign);
    bool parseTextAlign(const String&, TextAlign&);

    String textBaselineName(TextBaseline);
    bool parseTextBaseline(const String&, TextBaseline&);

///////////////////

class FloatRect;
class GraphicsContext;
class IntSize;

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/Generator.h

class Generator /*: public RefCounted<Generator>*/ {
public:
    virtual ~Generator() {};

    virtual void fill(GraphicsContext*, const FloatRect&) = 0;
    virtual void adjustParametersForTiledDrawing(IntSize& /* size */, FloatRect& /* srcRect */) { }
};

