#include "common.h"

#include "litehtml.h"
#include "color.h"

////////////////////////////////

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/GraphicsTypes.cpp

static const char* const compositeOperatorNames[] = {
    "clear",
    "copy",
    "source-over",
    "source-in",
    "source-out",
    "source-atop",
    "destination-over",
    "destination-in",
    "destination-out",
    "destination-atop",
    "xor",
    "darker",
    "highlight",
    "lighter"
};
const int numCompositeOperatorNames = sizeof(compositeOperatorNames) / sizeof(compositeOperatorNames[0]);

bool parseCompositeOperator(const String& s, CompositeOperator& op)
{
    for (int i = 0; i < numCompositeOperatorNames; i++)
        if (s == compositeOperatorNames[i]) {
            op = static_cast<CompositeOperator>(i);
            return true;
        }
    return false;
}

String compositeOperatorName(CompositeOperator op)
{
    ASSERT(op >= 0);
    ASSERT(op < numCompositeOperatorNames);
    return compositeOperatorNames[op];
}

bool parseLineCap(const String& s, LineCap& cap)
{
    if (s == "butt") {
        cap = ButtCap;
        return true;
    }
    if (s == "round") {
        cap = RoundCap;
        return true;
    }
    if (s == "square") {
        cap = SquareCap;
        return true;
    }
    return false;
}

String lineCapName(LineCap cap)
{
    ASSERT(cap >= 0);
    ASSERT(cap < 3);
    const char* const names[3] = { "butt", "round", "square" };
    return names[cap];
}

bool parseLineJoin(const String& s, LineJoin& join)
{
    if (s == "miter") {
        join = MiterJoin;
        return true;
    }
    if (s == "round") {
        join = RoundJoin;
        return true;
    }
    if (s == "bevel") {
        join = BevelJoin;
        return true;
    }
    return false;
}

String lineJoinName(LineJoin join)
{
    ASSERT(join >= 0);
    ASSERT(join < 3);
    const char* const names[3] = { "miter", "round", "bevel" };
    return names[join];
}

String textAlignName(TextAlign align)
{
    ASSERT(align >= 0);
    ASSERT(align < 5);
    const char* const names[5] = { "start", "end", "left", "center", "right" };
    return names[align];
}

bool parseTextAlign(const String& s, TextAlign& align)
{
    if (s == "start") {
        align = StartTextAlign;
        return true;
    }
    if (s == "end") {
        align = EndTextAlign;
        return true;
    }
    if (s == "left") {
        align = LeftTextAlign;
        return true;
    }
    if (s == "center") {
        align = CenterTextAlign;
        return true;
    }
    if (s == "right") {
        align = RightTextAlign;
        return true;
    }
    return false;
}

String textBaselineName(TextBaseline baseline)
{
    ASSERT(baseline >= 0);
    ASSERT(baseline < 6);
    const char* const names[6] = { "alphabetic", "top", "middle", "bottom", "ideographic", "hanging" };
    return names[baseline];
}

bool parseTextBaseline(const String& s, TextBaseline& baseline)
{
    if (s == "alphabetic") {
        baseline = AlphabeticTextBaseline;
        return true;
    }
    if (s == "top") {
        baseline = TopTextBaseline;
        return true;
    }
    if (s == "middle") {
        baseline = MiddleTextBaseline;
        return true;
    }
    if (s == "bottom") {
        baseline = BottomTextBaseline;
        return true;
    }
    if (s == "ideographic") {
        baseline = IdeographicTextBaseline;
        return true;
    }
    if (s == "hanging") {
        baseline = HangingTextBaseline;
        return true;
    }
    return false;
}

Color toColor(const litehtml::web_color &clr) {
  return Color(clr.red,clr.green,clr.blue,clr.alpha);
}
