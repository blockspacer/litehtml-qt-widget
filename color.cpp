#include "color.h"

//#include <math.h>
#include <cmath>
#include <fenv.h>

using namespace std;

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/ColorQt.cpp
Color::Color(const QColor& c)
    : m_color(makeRGBA(c.red(), c.green(), c.blue(), c.alpha()))
{
    m_valid = c.isValid();
}

Color::operator QColor() const
{
    if (m_valid)
        return QColor(red(), green(), blue(), alpha());
    else
        return QColor();
}

//////////////////////////

//#if !COMPILER(MSVC)
const RGBA32 Color::black;
const RGBA32 Color::white;
const RGBA32 Color::darkGray;
const RGBA32 Color::gray;
const RGBA32 Color::lightGray;
const RGBA32 Color::transparent;
//#endif

static const RGBA32 lightenedBlack = 0xFF545454;
static const RGBA32 darkenedWhite = 0xFFABABAB;

RGBA32 makeRGB(int r, int g, int b)
{
    return 0xFF000000 | max(0, min(r, 255)) << 16 | max(0, min(g, 255)) << 8 | max(0, min(b, 255));
}

RGBA32 makeRGBA(int r, int g, int b, int a)
{
    return max(0, min(a, 255)) << 24 | max(0, min(r, 255)) << 16 | max(0, min(g, 255)) << 8 | max(0, min(b, 255));
}

static int colorFloatToRGBAByte(float f)
{
    // We use lroundf and 255 instead of nextafterf(256, 0) to match CG's rounding
    return max(0, min(static_cast<int>(lroundf(255.0f * f)), 255));
}

RGBA32 makeRGBA32FromFloats(float r, float g, float b, float a)
{
    return colorFloatToRGBAByte(a) << 24 | colorFloatToRGBAByte(r) << 16 | colorFloatToRGBAByte(g) << 8 | colorFloatToRGBAByte(b);
}

RGBA32 colorWithOverrideAlpha(RGBA32 color, float overrideAlpha)
{
    RGBA32 rgbOnly = color & 0x00FFFFFF;
    RGBA32 rgba = rgbOnly | colorFloatToRGBAByte(overrideAlpha) << 24;
    return rgba;
}

static double calcHue(double temp1, double temp2, double hueVal)
{
    if (hueVal < 0.0)
        hueVal++;
    else if (hueVal > 1.0)
        hueVal--;
    if (hueVal * 6.0 < 1.0)
        return temp1 + (temp2 - temp1) * hueVal * 6.0;
    if (hueVal * 2.0 < 1.0)
        return temp2;
    if (hueVal * 3.0 < 2.0)
        return temp1 + (temp2 - temp1) * (2.0 / 3.0 - hueVal) * 6.0;
    return temp1;
}

// Explanation of this algorithm can be found in the CSS3 Color Module
// specification at http://www.w3.org/TR/css3-color/#hsl-color with further
// explanation available at http://en.wikipedia.org/wiki/HSL_color_space

// all values are in the range of 0 to 1.0
RGBA32 makeRGBAFromHSLA(double hue, double saturation, double lightness, double alpha)
{
    const double scaleFactor = nextafter(256.0, 0.0);

    if (!saturation) {
        int greyValue = static_cast<int>(lightness * scaleFactor);
        return makeRGBA(greyValue, greyValue, greyValue, static_cast<int>(alpha * scaleFactor));
    }

    double temp2 = lightness < 0.5 ? lightness * (1.0 + saturation) : lightness + saturation - lightness * saturation;
    double temp1 = 2.0 * lightness - temp2;

    return makeRGBA(static_cast<int>(calcHue(temp1, temp2, hue + 1.0 / 3.0) * scaleFactor),
                    static_cast<int>(calcHue(temp1, temp2, hue) * scaleFactor),
                    static_cast<int>(calcHue(temp1, temp2, hue - 1.0 / 3.0) * scaleFactor),
                    static_cast<int>(alpha * scaleFactor));
}

RGBA32 makeRGBAFromCMYKA(float c, float m, float y, float k, float a)
{
    double colors = 1 - k;
    int r = static_cast<int>(nextafter(256, 0) * (colors * (1 - c)));
    int g = static_cast<int>(nextafter(256, 0) * (colors * (1 - m)));
    int b = static_cast<int>(nextafter(256, 0) * (colors * (1 - y)));
    return makeRGBA(r, g, b, static_cast<float>(nextafter(256, 0) * a));
}

// originally moved here from the CSS parser
bool Color::parseHexColor(const UChar* name, unsigned length, RGBA32& rgb)
{
    if (length != 3 && length != 6)
        return false;
    unsigned value = 0;
    for (unsigned i = 0; i < length; ++i) {
        if (!isASCIIHexDigit(name[i]))
            return false;
        value <<= 4;
        value |= toASCIIHexValue(name[i]);
    }
    if (length == 6) {
        rgb = 0xFF000000 | value;
        return true;
    }
    // #abc converts to #aabbcc
    rgb = 0xFF000000
        | (value & 0xF00) << 12 | (value & 0xF00) << 8
        | (value & 0xF0) << 8 | (value & 0xF0) << 4
        | (value & 0xF) << 4 | (value & 0xF);
    return true;
}

bool Color::parseHexColor(const String& name, RGBA32& rgb)
{
    //return parseHexColor(name.characters(), name.length(), rgb);
    return parseHexColor(name.toStdString().c_str(), name.length(), rgb);
}

int differenceSquared(const Color& c1, const Color& c2)
{
    int dR = c1.red() - c2.red();
    int dG = c1.green() - c2.green();
    int dB = c1.blue() - c2.blue();
    return dR * dR + dG * dG + dB * dB;
}

Color::Color(const String& name)
{
    if (name[0] == '#')
        //m_valid = parseHexColor(name.characters() + 1, name.length() - 1, m_color);
        m_valid = parseHexColor(name.toStdString().c_str() + 1, name.length() - 1, m_color);
    /*else
        setNamedColor(name);*/
}

Color::Color(const char* name)
{
    if (name[0] == '#')
        m_valid = parseHexColor(&name[1], m_color);
    else {
        /*const NamedColor* foundColor = findColor(name, strlen(name));
        m_color = foundColor ? foundColor->RGBValue : 0;
        m_color |= 0xFF000000;
        m_valid = foundColor;*/
    }
}

/*String Color::serialized() const
{
    if (alpha() == 0xFF)
        return String::format("#%02x%02x%02x", red(), green(), blue());

    // Match Gecko ("0.0" for zero, 5 decimals for anything else)
    if (!alpha())
        return String::format("rgba(%u, %u, %u, 0.0)", red(), green(), blue());

    return String::format("rgba(%u, %u, %u, %.5f)", red(), green(), blue(), alpha() / 255.0f);
}

String Color::name() const
{
    if (alpha() < 0xFF)
        return String::format("#%02X%02X%02X%02X", red(), green(), blue(), alpha());
    return String::format("#%02X%02X%02X", red(), green(), blue());
}

static inline const NamedColor* findNamedColor(const String& name)
{
    char buffer[64]; // easily big enough for the longest color name
    unsigned length = name.length();
    if (length > sizeof(buffer) - 1)
        return 0;
    for (unsigned i = 0; i < length; ++i) {
        UChar c = name[i].toLatin1();//name[i]
        if (!c || c > 0x7F)
            return 0;
        buffer[i] = toASCIILower(static_cast<char>(c));
    }
    buffer[length] = '\0';
    return findColor(buffer, length);
}

void Color::setNamedColor(const String& name)
{
    const NamedColor* foundColor = findNamedColor(name);
    m_color = foundColor ? foundColor->RGBValue : 0;
    m_color |= 0xFF000000;
    m_valid = foundColor;
}*/

Color Color::light() const
{
    // Hardcode this common case for speed.
    if (m_color == black)
        return lightenedBlack;

    const float scaleFactor = nextafterf(256.0f, 0.0f);

    float r, g, b, a;
    getRGBA(r, g, b, a);

    float v = max(r, max(g, b));

    if (v == 0.0f)
        // Lightened black with alpha.
        return Color(0x54, 0x54, 0x54, alpha());

    float multiplier = min(1.0f, v + 0.33f) / v;

    return Color(static_cast<int>(multiplier * r * scaleFactor),
                 static_cast<int>(multiplier * g * scaleFactor),
                 static_cast<int>(multiplier * b * scaleFactor),
                 alpha());
}

Color Color::dark() const
{
    // Hardcode this common case for speed.
    if (m_color == white)
        return darkenedWhite;

    const float scaleFactor = nextafterf(256.0f, 0.0f);

    float r, g, b, a;
    getRGBA(r, g, b, a);

    float v = max(r, max(g, b));
    float multiplier = max(0.0f, (v - 0.33f) / v);

    return Color(static_cast<int>(multiplier * r * scaleFactor),
                 static_cast<int>(multiplier * g * scaleFactor),
                 static_cast<int>(multiplier * b * scaleFactor),
                 alpha());
}

static int blendComponent(int c, int a)
{
    // We use white.
    float alpha = a / 255.0f;
    int whiteBlend = 255 - a;
    c -= whiteBlend;
    return static_cast<int>(c / alpha);
}

const int cStartAlpha = 153; // 60%
const int cEndAlpha = 204; // 80%;
const int cAlphaIncrement = 17; // Increments in between.

Color Color::blend(const Color& source) const
{
    if (!alpha() || !source.hasAlpha())
        return source;

    if (!source.alpha())
        return *this;

    int d = 255 * (alpha() + source.alpha()) - alpha() * source.alpha();
    int a = d / 255;
    int r = (red() * alpha() * (255 - source.alpha()) + 255 * source.alpha() * source.red()) / d;
    int g = (green() * alpha() * (255 - source.alpha()) + 255 * source.alpha() * source.green()) / d;
    int b = (blue() * alpha() * (255 - source.alpha()) + 255 * source.alpha() * source.blue()) / d;
    return Color(r, g, b, a);
}

Color Color::blendWithWhite() const
{
    // If the color contains alpha already, we leave it alone.
    if (hasAlpha())
        return *this;

    Color newColor;
    for (int alpha = cStartAlpha; alpha <= cEndAlpha; alpha += cAlphaIncrement) {
        // We have a solid color.  Convert to an equivalent color that looks the same when blended with white
        // at the current alpha.  Try using less transparency if the numbers end up being negative.
        int r = blendComponent(red(), alpha);
        int g = blendComponent(green(), alpha);
        int b = blendComponent(blue(), alpha);

        newColor = Color(r, g, b, alpha);

        if (r >= 0 && g >= 0 && b >= 0)
            break;
    }
    return newColor;
}

void Color::getRGBA(float& r, float& g, float& b, float& a) const
{
    r = red() / 255.0f;
    g = green() / 255.0f;
    b = blue() / 255.0f;
    a = alpha() / 255.0f;
}

void Color::getRGBA(double& r, double& g, double& b, double& a) const
{
    r = red() / 255.0;
    g = green() / 255.0;
    b = blue() / 255.0;
    a = alpha() / 255.0;
}

void Color::getHSL(double& hue, double& saturation, double& lightness) const
{
    // http://en.wikipedia.org/wiki/HSL_color_space. This is a direct copy of
    // the algorithm therein, although it's 360^o based and we end up wanting
    // [0...1) based. It's clearer if we stick to 360^o until the end.
    double r = static_cast<double>(red()) / 255.0;
    double g = static_cast<double>(green()) / 255.0;
    double b = static_cast<double>(blue()) / 255.0;
    double max = std::max(std::max(r, g), b);
    double min = std::min(std::min(r, g), b);

    if (max == min)
        hue = 0.0;
    else if (max == r)
        hue = (60.0 * ((g - b) / (max - min))) + 360.0;
    else if (max == g)
        hue = (60.0 * ((b - r) / (max - min))) + 120.0;
    else
        hue = (60.0 * ((r - g) / (max - min))) + 240.0;

    if (hue >= 360.0)
        hue -= 360.0;

    // makeRGBAFromHSLA assumes that hue is in [0...1).
    hue /= 360.0;

    lightness = 0.5 * (max + min);
    if (max == min)
        saturation = 0.0;
    else if (lightness <= 0.5)
        saturation = ((max - min) / (max + min));
    else
        saturation = ((max - min) / (2.0 - (max + min)));
}

Color colorFromPremultipliedARGB(unsigned pixelColor)
{
    RGBA32 rgba;

    if (unsigned alpha = (pixelColor & 0xFF000000) >> 24) {
        rgba = makeRGBA(((pixelColor & 0x00FF0000) >> 16) * 255 / alpha,
                        ((pixelColor & 0x0000FF00) >> 8) * 255 / alpha,
                         (pixelColor & 0x000000FF) * 255 / alpha,
                          alpha);
    } else
        rgba = pixelColor;

    return Color(rgba);
}

unsigned premultipliedARGBFromColor(const Color& color)
{
    unsigned pixelColor;

    if (unsigned alpha = color.alpha()) {
        pixelColor = alpha << 24 |
             ((color.red() * alpha  + 254) / 255) << 16 |
             ((color.green() * alpha  + 254) / 255) << 8 |
             ((color.blue() * alpha  + 254) / 255);
    } else
         pixelColor = color.rgb();

    return pixelColor;
}

////////////////////

/*
#if !((' ' == 32) && ('!' == 33) && ('"' == 34) && ('#' == 35) \
      && ('%' == 37) && ('&' == 38) && ('\'' == 39) && ('(' == 40) \
      && (')' == 41) && ('*' == 42) && ('+' == 43) && (',' == 44) \
      && ('-' == 45) && ('.' == 46) && ('/' == 47) && ('0' == 48) \
      && ('1' == 49) && ('2' == 50) && ('3' == 51) && ('4' == 52) \
      && ('5' == 53) && ('6' == 54) && ('7' == 55) && ('8' == 56) \
      && ('9' == 57) && (':' == 58) && (';' == 59) && ('<' == 60) \
      && ('=' == 61) && ('>' == 62) && ('?' == 63) && ('A' == 65) \
      && ('B' == 66) && ('C' == 67) && ('D' == 68) && ('E' == 69) \
      && ('F' == 70) && ('G' == 71) && ('H' == 72) && ('I' == 73) \
      && ('J' == 74) && ('K' == 75) && ('L' == 76) && ('M' == 77) \
      && ('N' == 78) && ('O' == 79) && ('P' == 80) && ('Q' == 81) \
      && ('R' == 82) && ('S' == 83) && ('T' == 84) && ('U' == 85) \
      && ('V' == 86) && ('W' == 87) && ('X' == 88) && ('Y' == 89) \
      && ('Z' == 90) && ('[' == 91) && ('\\' == 92) && (']' == 93) \
      && ('^' == 94) && ('_' == 95) && ('a' == 97) && ('b' == 98) \
      && ('c' == 99) && ('d' == 100) && ('e' == 101) && ('f' == 102) \
      && ('g' == 103) && ('h' == 104) && ('i' == 105) && ('j' == 106) \
      && ('k' == 107) && ('l' == 108) && ('m' == 109) && ('n' == 110) \
      && ('o' == 111) && ('p' == 112) && ('q' == 113) && ('r' == 114) \
      && ('s' == 115) && ('t' == 116) && ('u' == 117) && ('v' == 118) \
      && ('w' == 119) && ('x' == 120) && ('y' == 121) && ('z' == 122) \
      && ('{' == 123) && ('|' == 124) && ('}' == 125) && ('~' == 126))

#error "gperf generated tables don't work with this execution character set. Please report a bug to <bug-gnu-gperf@gnu.org>."
#endif

// https://github.com/trevorlinton/webkit.js/blob/15306b5c540ccafc3be008225eafebd1932d356d/src/WebCoreDerived/ColorData.cpp#L42
enum
{
  TOTAL_KEYWORDS = 150,
  MIN_WORD_LENGTH = 3,
  MAX_WORD_LENGTH = 20,
  MIN_HASH_VALUE = 3,
  MAX_HASH_VALUE = 1055
};

class ColorDataHash
{
private:
  static inline unsigned int colordata_hash_function (const char *str, unsigned int len);
public:
  static const struct NamedColor *findColorImpl (const char *str, unsigned int len);
};


inline unsigned int
ColorDataHash::colordata_hash_function (register const char *str, register unsigned int len)
{
  static const unsigned short asso_values[] =
    {
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056,    5,    0,  105,
         0,    0,   30,   40,   40,   10,    0,    0,   15,   60,
         0,    5,  255,   40,    0,   10,   15,  130,  300,  215,
         5,    0,    0, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056, 1056,
      1056, 1056, 1056, 1056, 1056, 1056, 1056
    };
  register int hval = (int)len;

  switch (hval)
    {
      default:
        hval += asso_values[(unsigned char)str[19]];

      case 19:
        hval += asso_values[(unsigned char)str[18]];

      case 18:
        hval += asso_values[(unsigned char)str[17]];

      case 17:
        hval += asso_values[(unsigned char)str[16]];

      case 16:
        hval += asso_values[(unsigned char)str[15]];

      case 15:
        hval += asso_values[(unsigned char)str[14]];

      case 14:
        hval += asso_values[(unsigned char)str[13]];

      case 13:
        hval += asso_values[(unsigned char)str[12]];

      case 12:
        hval += asso_values[(unsigned char)str[11]];

      case 11:
        hval += asso_values[(unsigned char)str[10]+1];

      case 10:
        hval += asso_values[(unsigned char)str[9]];

      case 9:
        hval += asso_values[(unsigned char)str[8]];

      case 8:
        hval += asso_values[(unsigned char)str[7]];

      case 7:
        hval += asso_values[(unsigned char)str[6]];

      case 6:
        hval += asso_values[(unsigned char)str[5]];

      case 5:
        hval += asso_values[(unsigned char)str[4]];

      case 4:
        hval += asso_values[(unsigned char)str[3]];

      case 3:
        hval += asso_values[(unsigned char)str[2]];

      case 2:
        hval += asso_values[(unsigned char)str[1]];

      case 1:
        hval += asso_values[(unsigned char)str[0]];
        break;
    }
  return hval;
}

static const struct NamedColor wordlist[] =
  {
#line 145 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"red", 0xffff0000},
#line 57 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkred", 0xff8b0000},
#line 162 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"tan", 0xffd2b48c},
#line 110 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"linen", 0xfffaf0e6},
#line 153 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"sienna", 0xffa0522d},
#line 84 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"indianred", 0xffcd5c5c},
#line 163 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"teal", 0xff008080},
#line 79 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"grey", 0xff808080},
#line 80 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"green", 0xff008000},
#line 78 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"gray", 0xff808080},
#line 50 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkgrey", 0xffa9a9a9},
#line 51 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkgreen", 0xff006400},
#line 30 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"beige", 0xfff5f5dc},
#line 131 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"orange", 0xffffa500},
#line 49 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkgray", 0xffa9a9a9},
#line 132 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"orangered", 0xffff4500},
#line 87 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"khaki", 0xfff0e68c},
#line 151 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"seagreen", 0xff2e8b57},
#line 76 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"gold", 0xffffd700},
#line 55 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkorange", 0xffff8c00},
#line 52 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkkhaki", 0xffbdb76b},
#line 85 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"indigo", 0xff4b0082},
#line 77 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"goldenrod", 0xffdaa520},
#line 112 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"maroon", 0xff800000},
#line 74 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"gainsboro", 0xffdcdcdc},
#line 108 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lime", 0xff00ff00},
#line 81 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"greenyellow", 0xffadff2f},
#line 48 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkgoldenrod", 0xffb8860b},
#line 158 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"slategrey", 0xff708090},
#line 157 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"slategray", 0xff708090},
#line 149 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"salmon", 0xfffa8072},
#line 59 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkseagreen", 0xff8fbc8f},
#line 152 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"seashell", 0xfffff5ee},
#line 58 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darksalmon", 0xffe9967a},
#line 165 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"tomato", 0xffff6347},
#line 164 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"thistle", 0xffd8bfd8},
#line 62 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkslategrey", 0xff2f4f4f},
#line 45 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"cyan", 0xff00ffff},
#line 72 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"forestgreen", 0xff228b22},
#line 68 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"dimgrey", 0xff696969},
#line 61 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkslategray", 0xff2f4f4f},
#line 124 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mistyrose", 0xffffe4e1},
#line 67 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"dimgray", 0xff696969},
#line 47 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkcyan", 0xff008b8b},
#line 32 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"black", 0xff000000},
#line 111 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"magenta", 0xffff00ff},
#line 109 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"limegreen", 0xff32cd32},
#line 41 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"coral", 0xffff7f50},
#line 53 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkmagenta", 0xff8b008b},
#line 29 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"azure", 0xfff0ffff},
#line 34 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"blue", 0xff0000ff},
#line 128 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"oldlace", 0xfffdf5e6},
#line 43 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"cornsilk", 0xfffff8dc},
#line 46 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkblue", 0xff00008b},
#line 155 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"skyblue", 0xff87ceeb},
#line 70 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"firebrick", 0xffb22222},
#line 133 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"orchid", 0xffda70d6},
#line 97 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightgrey", 0xffd3d3d3},
#line 98 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightgreen", 0xff90ee90},
#line 107 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightyellow", 0xffffffe0},
#line 96 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightgray", 0xffd3d3d3},
#line 56 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkorchid", 0xff9932cc},
#line 147 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"royalblue", 0xff4169e1},
#line 27 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"aqua", 0xff00ffff},
#line 161 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"steelblue", 0xff4682b4},
#line 31 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"bisque", 0xffffe4c4},
#line 44 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"crimson", 0xffdc143c},
#line 156 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"slateblue", 0xff6a5acd},
#line 69 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"dodgerblue", 0xff1e90ff},
#line 33 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"blanchedalmond", 0xffffebcd},
#line 101 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightseagreen", 0xff20b2aa},
#line 105 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightslategrey", 0xff778899},
#line 104 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightslategray", 0xff778899},
#line 36 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"brown", 0xffa52a2a},
#line 100 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightsalmon", 0xffffa07a},
#line 159 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"snow", 0xfffffafa},
#line 94 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightcyan", 0xffe0ffff},
#line 146 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"rosybrown", 0xffbc8f8f},
#line 150 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"sandybrown", 0xfff4a460},
#line 60 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkslateblue", 0xff483d8b},
#line 173 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"yellow", 0xffffff00},
#line 93 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightcoral", 0xfff08080},
#line 123 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mintcream", 0xfff5fffa},
#line 28 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"aquamarine", 0xff7fffd4},
#line 148 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"saddlebrown", 0xff8b4513},
#line 82 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"honeydew", 0xfff0fff0},
#line 141 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"pink", 0xffffc0cb},
#line 92 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightblue", 0xffadd8e6},
#line 38 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"cadetblue", 0xff5f9ea0},
#line 170 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"wheat", 0xfff5deb3},
#line 90 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lawngreen", 0xff7cfc00},
#line 171 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"white", 0xffffffff},
#line 25 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"aliceblue", 0xfff0f8ff},
#line 40 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"chocolate", 0xffd2691e},
#line 174 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"yellowgreen", 0xff9acd32},
#line 125 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"moccasin", 0xffffe4b5},
#line 127 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"navy", 0xff000080},
#line 39 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"chartreuse", 0xff7fff00},
#line 86 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"ivory", 0xfffffff0},
#line 135 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"palegreen", 0xff98fb98},
#line 88 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lavender", 0xffe6e6fa},
#line 83 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"hotpink", 0xffff69b4},
#line 129 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"olive", 0xff808000},
#line 73 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"fuchsia", 0xffff00ff},
#line 117 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumseagreen", 0xff3cb371},
#line 154 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"silver", 0xffc0c0c0},
#line 130 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"olivedrab", 0xff6b8e23},
#line 63 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkturquoise", 0xff00ced1},
#line 167 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"turquoise", 0xff40e0d0},
#line 168 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"violet", 0xffee82ee},
#line 169 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"violetred", 0xffd02090},
#line 64 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkviolet", 0xff9400d3},
#line 134 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"palegoldenrod", 0xffeee8aa},
#line 172 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"whitesmoke", 0xfff5f5f5},
#line 160 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"springgreen", 0xff00ff7f},
#line 37 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"burlywood", 0xffdeb887},
#line 140 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"peru", 0xffcd853f},
#line 71 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"floralwhite", 0xfffffaf0},
#line 99 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightpink", 0xffffb6c1},
#line 54 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"darkolivegreen", 0xff556b2f},
#line 75 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"ghostwhite", 0xfff8f8ff},
#line 114 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumblue", 0xff0000cd},
#line 115 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumorchid", 0xffba55d3},
#line 106 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightsteelblue", 0xffb0c4de},
#line 103 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightslateblue", 0xff8470ff},
#line 166 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"transparent", 0x00000000},
#line 66 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"deepskyblue", 0xff00bfff},
#line 102 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightskyblue", 0xff87cefa},
#line 95 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lightgoldenrodyellow", 0xfffafad2},
#line 142 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"plum", 0xffdda0dd},
#line 113 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumaquamarine", 0xff66cdaa},
#line 118 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumslateblue", 0xff7b68ee},
#line 35 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"blueviolet", 0xff8a2be2},
#line 122 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"midnightblue", 0xff191970},
#line 65 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"deeppink", 0xffff1493},
#line 91 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lemonchiffon", 0xfffffacd},
#line 26 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"antiquewhite", 0xfffaebd7},
#line 136 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"paleturquoise", 0xffafeeee},
#line 143 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"powderblue", 0xffb0e0e6},
#line 126 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"navajowhite", 0xffffdead},
#line 119 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumspringgreen", 0xff00fa9a},
#line 42 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"cornflowerblue", 0xff6495ed},
#line 137 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"palevioletred", 0xffdb7093},
#line 121 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumvioletred", 0xffc71585},
#line 144 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"purple", 0xff800080},
#line 89 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"lavenderblush", 0xfffff0f5},
#line 120 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumturquoise", 0xff48d1cc},
#line 139 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"peachpuff", 0xffffdab9},
#line 116 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"mediumpurple", 0xff9370db},
#line 138 "/Tint/webkit.js/tools/../deps/WebKit/Source/WebCore/platform/ColorData.gperf"
    {"papayawhip", 0xffffefd5}
  };

static const short lookup[] =
  {
     -1,  -1,  -1,   0,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,   1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,   2,  -1,  -1,  -1,  -1,  -1,  -1,
      3,   4,  -1,  -1,   5,  -1,  -1,  -1,  -1,   6,
     -1,  -1,  -1,  -1,   7,   8,  -1,  -1,  -1,   9,
     -1,  -1,  -1,  10,  11,  12,  13,  -1,  14,  15,
     16,  -1,  -1,  17,  18,  19,  -1,  -1,  -1,  20,
     -1,  21,  -1,  -1,  22,  -1,  -1,  -1,  -1,  -1,
     -1,  23,  -1,  -1,  24,  -1,  -1,  -1,  -1,  25,
     -1,  26,  -1,  27,  28,  -1,  -1,  -1,  -1,  29,
     -1,  30,  31,  32,  -1,  -1,  -1,  -1,  -1,  -1,
     33,  34,  35,  36,  37,  -1,  38,  39,  40,  41,
     -1,  -1,  42,  43,  -1,  -1,  -1,  -1,  -1,  -1,
     44,  -1,  45,  -1,  46,  47,  48,  -1,  -1,  -1,
     49,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  50,
     -1,  -1,  51,  52,  -1,  -1,  -1,  -1,  53,  -1,
     -1,  -1,  54,  -1,  55,  -1,  56,  -1,  -1,  57,
     58,  59,  -1,  -1,  60,  61,  -1,  -1,  -1,  62,
     -1,  -1,  -1,  -1,  63,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  64,  -1,  65,  66,  -1,  67,
     68,  -1,  -1,  -1,  69,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  70,  71,
     -1,  -1,  -1,  -1,  72,  73,  -1,  -1,  -1,  -1,
     -1,  74,  -1,  -1,  75,  -1,  -1,  -1,  -1,  76,
     -1,  -1,  -1,  -1,  77,  78,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  79,  -1,  -1,  80,  -1,  -1,  -1,
     81,  -1,  -1,  -1,  82,  83,  84,  -1,  85,  86,
     -1,  -1,  -1,  -1,  87,  -1,  -1,  -1,  -1,  88,
     89,  -1,  -1,  -1,  90,  91,  -1,  -1,  -1,  92,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  93,  -1,  94,  -1,  95,  96,
     -1,  -1,  -1,  -1,  -1,  97,  -1,  -1,  -1,  -1,
     98,  -1,  -1,  -1,  99,  -1,  -1,  -1, 100,  -1,
     -1,  -1, 101,  -1,  -1, 102,  -1, 103,  -1, 104,
     -1, 105,  -1,  -1, 106,  -1,  -1,  -1, 107, 108,
     -1, 109,  -1,  -1, 110,  -1,  -1,  -1,  -1,  -1,
    111,  -1,  -1, 112,  -1, 113,  -1,  -1,  -1,  -1,
     -1, 114,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 115,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 116,
     -1, 117,  -1,  -1, 118,  -1,  -1,  -1,  -1, 119,
    120,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1, 121,  -1,  -1,  -1,  -1,
     -1,  -1, 122,  -1, 123,  -1,  -1,  -1,  -1, 124,
     -1, 125,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1, 126,  -1,  -1,  -1,  -1,  -1, 127,  -1,  -1,
    128,  -1,  -1,  -1, 129,  -1,  -1,  -1,  -1,  -1,
     -1, 130,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1, 131,  -1,  -1,  -1,  -1,
    132,  -1, 133,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 134,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1, 135,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1, 136,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 137,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
    138,  -1,  -1,  -1,  -1,  -1, 139, 140,  -1, 141,
     -1,  -1,  -1, 142,  -1,  -1,  -1,  -1,  -1,  -1,
    143,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1, 144,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 145,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1, 146,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1, 147,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1, 148,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,  -1,
     -1,  -1,  -1,  -1,  -1, 149
  };

const struct NamedColor *
ColorDataHash::findColorImpl (register const char *str, register unsigned int len)
{
  if (len <= MAX_WORD_LENGTH && len >= MIN_WORD_LENGTH)
    {
      register int key = colordata_hash_function (str, len);

      if (key <= MAX_HASH_VALUE && key >= 0)
        {
          register int index = lookup[key];

          if (index >= 0)
            {
              register const char *s = wordlist[index].name;

              if (*str == *s && !strncmp (str + 1, s + 1, len - 1) && s[len] == '\0')
                return &wordlist[index];
            }
        }
    }
  return 0;
}

const struct NamedColor* findColor(const char* str, unsigned int len)
{
    return ColorDataHash::findColorImpl(str, len);
}*/
