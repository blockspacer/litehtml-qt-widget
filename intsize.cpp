#include "intsize.h"

#include <QSize>

/*IntSize::IntSize()
{

}*/

// https://github.com/rkudiyarov/ClutterWebkit/blob/05d919e0598691bcd34f57d27f44872919e39e92/WebCore/platform/graphics/qt/IntSizeQt.cpp
IntSize::IntSize(const QSize& r)
    : m_width(r.width())
    , m_height(r.height())
{
}

IntSize::operator QSize() const
{
    return QSize(width(), height());
}
