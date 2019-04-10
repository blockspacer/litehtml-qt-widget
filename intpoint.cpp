#include "intpoint.h"

#include <QPoint>

/*IntPoint::IntPoint()
{

}*/

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/IntPointQt.cpp
IntPoint::IntPoint(const QPoint& p)
    : m_x(p.x())
    , m_y(p.y())
{
}

IntPoint::operator QPoint() const
{
    return QPoint(m_x, m_y);
}
