#pragma once

#include "common.h"

#include "affinetransform.h"
#include "floatpoint.h"
//#include "graphicscontext.h"
#include "intsize.h"
#include "floatrect.h"

//#include "Generator.h"
//#include "GraphicsTypes.h"
//#include <wtf/PassRefPtr.h>
//#include <wtf/Vector.h>

QT_BEGIN_NAMESPACE
class QGradient;
QT_END_NAMESPACE
typedef QGradient* PlatformGradient;

    class Color;

    class Gradient : public Generator {
    public:
        static PassRefPtr<Gradient> create(const FloatPoint& p0, const FloatPoint& p1)
        {
            //return adoptRef(new Gradient(p0, p1));
            return adoptRef<Gradient>(p0, p1);
        }
        static PassRefPtr<Gradient> create(const FloatPoint& p0, float r0, const FloatPoint& p1, float r1)
        {
            //return adoptRef(new Gradient(p0, r0, p1, r1));
            return adoptRef<Gradient>(p0, r0, p1, r1);
        }
        virtual ~Gradient();

        struct ColorStop;
        void addColorStop(const ColorStop&);
        void addColorStop(float, const Color&);

        void getColor(float value, float* r, float* g, float* b, float* a) const;

        bool isRadial() const { return m_radial; }
        bool isZeroSize() const { return m_p0.x() == m_p1.x() && m_p0.y() == m_p1.y() && (!m_radial || m_r0 == m_r1); }

        PlatformGradient platformGradient();

        struct ColorStop {
            float stop;
            float red;
            float green;
            float blue;
            float alpha;

            ColorStop() : stop(0), red(0), green(0), blue(0), alpha(0) { }
            ColorStop(float s, float r, float g, float b, float a) : stop(s), red(r), green(g), blue(b), alpha(a) { }
        };

        void setStopsSorted(bool s) { m_stopsSorted = s; }

        void setSpreadMethod(GradientSpreadMethod);
        GradientSpreadMethod spreadMethod() { return m_spreadMethod; }
        void setGradientSpaceTransform(const AffineTransform& gradientSpaceTransformation);
        // Qt and CG transform the gradient at draw time
        AffineTransform gradientSpaceTransform() { return m_gradientSpaceTransformation; }

        virtual void fill(GraphicsContext*, const FloatRect&);
        virtual void adjustParametersForTiledDrawing(IntSize& size, FloatRect& srcRect);

        void setPlatformGradientSpaceTransform(const AffineTransform& gradientSpaceTransformation);

    //private:
    public:
        Gradient(const FloatPoint& p0, const FloatPoint& p1);
        Gradient(const FloatPoint& p0, float r0, const FloatPoint& p1, float r1);
    private:
        void platformInit() { m_gradient = 0; }
        void platformDestroy();

        int findStop(float value) const;
        void sortStopsIfNecessary();

        bool m_radial;
        FloatPoint m_p0;
        FloatPoint m_p1;
        float m_r0;
        float m_r1;

        //mutable Vector<ColorStop, 2> m_stops;
        mutable QVector<ColorStop> m_stops;

        mutable bool m_stopsSorted;
        mutable int m_lastStop;
        GradientSpreadMethod m_spreadMethod;
        AffineTransform m_gradientSpaceTransformation;

        PlatformGradient m_gradient;
    };
