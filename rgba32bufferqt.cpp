#include "rgba32bufferqt.h"

#include "common.h"
#include "imagedecoder.h"

#include <QPixmap>
#include <cstdio>


RGBA32Buffer::RGBA32Buffer()
    : m_hasAlpha(false)
    , m_size()
    , m_status(FrameEmpty)
    , m_duration(0)
    , m_disposalMethod(DisposeNotSpecified)
{
}

RGBA32Buffer& RGBA32Buffer::operator=(const RGBA32Buffer& other)
{
    if (this == &other)
        return *this;

    copyBitmapData(other);
    setRect(other.rect());
    setStatus(other.status());
    setDuration(other.duration());
    setDisposalMethod(other.disposalMethod());
    return *this;
}

void RGBA32Buffer::clear()
{
    m_pixmap = QPixmap();
    m_image = QImage();
    m_status = FrameEmpty;
    // NOTE: Do not reset other members here; clearFrameBufferCache()
    // calls this to free the bitmap data, but other functions like
    // initFrameBuffer() and frameComplete() may still need to read
    // other metadata out of this frame later.
}

void RGBA32Buffer::zeroFill()
{
    if (m_pixmap.isNull() && !m_image.isNull()) {
        m_pixmap = QPixmap(m_image.width(), m_image.height());
        m_image = QImage();
    }
    m_pixmap.fill(QColor(0, 0, 0, 0));
}

bool RGBA32Buffer::copyBitmapData(const RGBA32Buffer& other)
{
    if (this == &other)
        return true;

    m_image = other.m_image;
    m_pixmap = other.m_pixmap;
    m_size = other.m_size;
    m_hasAlpha = other.m_hasAlpha;
    return true;
}

bool RGBA32Buffer::setSize(int newWidth, int newHeight)
{
    // This function should only be called once, it will leak memory
    // otherwise.
    ASSERT(width() == 0 && height() == 0);

    m_size = IntSize(newWidth, newHeight);
    m_image = QImage();
    m_pixmap = QPixmap(newWidth, newHeight);
    if (m_pixmap.isNull())
        return false;

    // Zero the image.
    zeroFill();

    return true;
}

QPixmap* RGBA32Buffer::asNewNativeImage() const
{
    if (m_pixmap.isNull() && !m_image.isNull()) {
        m_pixmap = QPixmap::fromImage(m_image);
        m_image = QImage();
    }
    return new QPixmap(m_pixmap);
}

bool RGBA32Buffer::hasAlpha() const
{
    return m_hasAlpha;
}

void RGBA32Buffer::setHasAlpha(bool alpha)
{
    m_hasAlpha = alpha;
}

void RGBA32Buffer::setStatus(FrameStatus status)
{
    m_status = status;
}

// The image must not have format 8888 pre multiplied...
void RGBA32Buffer::setPixmap(const QPixmap& pixmap)
{
    m_pixmap = pixmap;
    m_image = QImage();
    m_size = pixmap.size();
    m_hasAlpha = pixmap.hasAlphaChannel();
}

int RGBA32Buffer::width() const
{
    return m_size.width();
}

int RGBA32Buffer::height() const
{
    return m_size.height();
}
