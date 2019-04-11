#pragma once

#include "common.h"

#include "imagedecoder.h"
#include <QtGui/QImageReader>
#include <QtGui/QPixmap>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QBuffer>
//#include <wtf/OwnPtr.h>

class ImageDecoderQt : public ImageDecoder
{
public:
    ImageDecoderQt(bool premultiplyAlpha);
    ~ImageDecoderQt();

    virtual void setData(SharedBuffer* data, bool allDataReceived);
    virtual bool isSizeAvailable();
    virtual size_t frameCount();
    virtual int repetitionCount() const;
    virtual RGBA32Buffer* frameBufferAtIndex(size_t index);

    virtual String filenameExtension() const;

    virtual void clearFrameBufferCache(size_t clearBeforeFrame);

private:
    ImageDecoderQt(const ImageDecoderQt&);
    ImageDecoderQt &operator=(const ImageDecoderQt&);

private:
    void internalDecodeSize();
    void internalReadImage(size_t);
    bool internalHandleCurrentImage(size_t);
    void forceLoadEverything();
    void clearPointers();

private:
    QByteArray m_format;
    OwnPtr<QBuffer> m_buffer;
    OwnPtr<QImageReader> m_reader;
    mutable int m_repetitionCount;
};
