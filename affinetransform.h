#pragma once

#include "transformationmatrix.h"

#include "common.h"

#include <string.h> // for memcpy

#include <QTransform>

#include <memory>
#include <stdio.h>
#include <string.h>

#include "floatquad.h"

using namespace std;

/*#include <wtf/FastAllocBase.h>

#if PLATFORM(CG)
#include <CoreGraphics/CGAffineTransform.h>
#elif PLATFORM(CAIRO)
#include <cairo.h>
#elif PLATFORM(OPENVG)
#include "VGUtils.h"
#elif PLATFORM(QT)
#include <QTransform>
#elif PLATFORM(SKIA)
#include <SkMatrix.h>
#elif PLATFORM(WX) && USE(WXGC)
#include <wx/graphics.h>
#endif*/

/*class FloatPoint;
class FloatQuad;
class FloatRect;
class IntPoint;
class IntRect;
class TransformationMatrix;*/

class AffineTransform /*: public FastAllocBase*/ {
public:
    typedef double Transform[6];

    AffineTransform();
    AffineTransform(double a, double b, double c, double d, double e, double f);

    void setMatrix(double a, double b, double c, double d, double e, double f);

    void map(double x, double y, double& x2, double& y2) const;

    // Rounds the mapped point to the nearest integer value.
    IntPoint mapPoint(const IntPoint&) const;

    FloatPoint mapPoint(const FloatPoint&) const;

    // Rounds the resulting mapped rectangle out. This is helpful for bounding
    // box computations but may not be what is wanted in other contexts.
    IntRect mapRect(const IntRect&) const;

    FloatRect mapRect(const FloatRect&) const;
    FloatQuad mapQuad(const FloatQuad&) const;

    bool isIdentity() const;

    double a() const { return m_transform[0]; }
    void setA(double a) { m_transform[0] = a; }
    double b() const { return m_transform[1]; }
    void setB(double b) { m_transform[1] = b; }
    double c() const { return m_transform[2]; }
    void setC(double c) { m_transform[2] = c; }
    double d() const { return m_transform[3]; }
    void setD(double d) { m_transform[3] = d; }
    double e() const { return m_transform[4]; }
    void setE(double e) { m_transform[4] = e; }
    double f() const { return m_transform[5]; }
    void setF(double f) { m_transform[5] = f; }

    void makeIdentity();

    AffineTransform& multiply(const AffineTransform&);
    AffineTransform& multLeft(const AffineTransform&);
    AffineTransform& scale(double);
    AffineTransform& scale(double sx, double sy);
    AffineTransform& scaleNonUniform(double sx, double sy);
    AffineTransform& rotate(double d);
    AffineTransform& rotateFromVector(double x, double y);
    AffineTransform& translate(double tx, double ty);
    AffineTransform& translateRight(double tx, double ty);
    AffineTransform& shear(double sx, double sy);
    AffineTransform& flipX();
    AffineTransform& flipY();
    AffineTransform& skew(double angleX, double angleY);
    AffineTransform& skewX(double angle);
    AffineTransform& skewY(double angle);

    double xScale() const;
    double yScale() const;

    double det() const;
    bool isInvertible() const;
    AffineTransform inverse() const;

    void blend(const AffineTransform& from, double progress);

    TransformationMatrix toTransformationMatrix() const;

    bool isIdentityOrTranslation() const
    {
        return m_transform[0] == 1 && m_transform[1] == 0 && m_transform[2] == 0 && m_transform[3] == 1;
    }

    bool isIdentityOrTranslationOrFlipped() const
    {
        return m_transform[0] == 1 && m_transform[1] == 0 && m_transform[2] == 0 && (m_transform[3] == 1 || m_transform[3] == -1);
    }

    bool operator== (const AffineTransform& m2) const
    {
        return (m_transform[0] == m2.m_transform[0]
             && m_transform[1] == m2.m_transform[1]
             && m_transform[2] == m2.m_transform[2]
             && m_transform[3] == m2.m_transform[3]
             && m_transform[4] == m2.m_transform[4]
             && m_transform[5] == m2.m_transform[5]);
    }

    bool operator!=(const AffineTransform& other) const { return !(*this == other); }

    // *this = *this * t (i.e., a multRight)
    AffineTransform& operator*=(const AffineTransform& t)
    {
        *this = *this * t;
        return *this;
    }

    // result = *this * t (i.e., a multRight)
    AffineTransform operator*(const AffineTransform& t) const
    {
        AffineTransform result = t;
        result.multLeft(*this);
        return result;
    }

  operator QTransform() const;

/*#if PLATFORM(CG)
    operator CGAffineTransform() const;
#elif PLATFORM(CAIRO)
    operator cairo_matrix_t() const;
#elif PLATFORM(OPENVG)
    operator VGMatrix() const;
#elif PLATFORM(QT)
    operator QTransform() const;
#elif PLATFORM(SKIA)
    operator SkMatrix() const;
#elif PLATFORM(WX) && USE(WXGC)
    operator wxGraphicsMatrix() const;
#endif*/

private:
    void setMatrix(const Transform m)
    {
        if (m && m != m_transform)
            memcpy(m_transform, m, sizeof(Transform));
    }

    Transform m_transform;
};

AffineTransform makeMapBetweenRects(const FloatRect& source, const FloatRect& dest);


