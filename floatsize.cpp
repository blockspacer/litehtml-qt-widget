#include "floatsize.h"

#include "common.h"

#include "intsize.h"
#include <cmath>

FloatSize::FloatSize(const IntSize& size) : m_width(size.width()), m_height(size.height())
{
}

float FloatSize::diagonalLength() const
{
    return sqrtf(diagonalLengthSquared());
}

FloatSize FloatSize::narrowPrecision(double width, double height)
{
    return FloatSize(narrowPrecisionToFloat(width), narrowPrecisionToFloat(height));
}
