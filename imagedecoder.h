#pragma once

#include "common.h"

#include "intrect.h"
#include "imagesource.h"
//#include "PlatformString.h"
#include "sharedbuffer.h"
//#include <wtf/Assertions.h>
//#include <wtf/RefPtr.h>
//#include <wtf/Vector.h>

/*#include "IntRect.h"
#include "ImageSource.h"
#include "PlatformString.h"
#include "SharedBuffer.h"
#include <wtf/Assertions.h>
#include <wtf/RefPtr.h>
#include <wtf/Vector.h>*/

#include <QPixmap>
#include <QImage>

    // The RGBA32Buffer object represents the decoded image data in RGBA32
    // format.  This buffer is what all decoders write a single frame into.
    // Frames are then instantiated for drawing by being handed this buffer.
    class RGBA32Buffer {
    public:
        enum FrameStatus { FrameEmpty, FramePartial, FrameComplete };
        enum FrameDisposalMethod {
            // If you change the numeric values of these, make sure you audit
            // all users, as some users may cast raw values to/from these
            // constants.
            DisposeNotSpecified,      // Leave frame in framebuffer
            DisposeKeep,              // Leave frame in framebuffer
            DisposeOverwriteBgcolor,  // Clear frame to transparent
            DisposeOverwritePrevious, // Clear frame to previous framebuffer
                                      // contents
        };

        typedef uint32_t PixelData;

        RGBA32Buffer();

        RGBA32Buffer(const RGBA32Buffer& other) { operator=(other); }

        // For backends which refcount their data, this operator doesn't need to
        // create a new copy of the image data, only increase the ref count.
        RGBA32Buffer& operator=(const RGBA32Buffer& other);

        // Deletes the pixel data entirely; used by ImageDecoder to save memory
        // when we no longer need to display a frame and only need its metadata.
        void clear();

        // Zeroes the pixel data in the buffer, setting it to fully-transparent.
        void zeroFill();

        // Creates a new copy of the image data in |other|, so the two images
        // can be modified independently.  Returns whether the copy succeeded.
        bool copyBitmapData(const RGBA32Buffer& other);

        // Copies the pixel data at [(startX, startY), (endX, startY)) to the
        // same X-coordinates on each subsequent row up to but not including
        // endY.
        void copyRowNTimes(int startX, int endX, int startY, int endY)
        {
            ASSERT(startX < width());
            ASSERT(endX <= width());
            ASSERT(startY < height());
            ASSERT(endY <= height());
            const int rowBytes = (endX - startX) * sizeof(PixelData);
            const PixelData* const startAddr = getAddr(startX, startY);
            for (int destY = startY + 1; destY < endY; ++destY)
                memcpy(getAddr(startX, destY), startAddr, rowBytes);
        }

        // Allocates space for the pixel data.  Must be called before any pixels
        // are written. Will return true on success, false if the memory
        // allocation fails.  Calling this multiple times is undefined and may
        // leak memory.
        bool setSize(int newWidth, int newHeight);

        // To be used by ImageSource::createFrameAtIndex().  Returns a pointer
        // to the underlying native image data.  This pointer will be owned by
        // the BitmapImage and freed in FrameData::clear().
        NativeImagePtr asNewNativeImage() const;

        bool hasAlpha() const;
        const IntRect& rect() const { return m_rect; }
        FrameStatus status() const { return m_status; }
        unsigned duration() const { return m_duration; }
        FrameDisposalMethod disposalMethod() const { return m_disposalMethod; }
        bool premultiplyAlpha() const { return m_premultiplyAlpha; }

        void setHasAlpha(bool alpha);
        void setRect(const IntRect& r) { m_rect = r; }
        void setStatus(FrameStatus status);
        void setDuration(unsigned duration) { m_duration = duration; }
        void setDisposalMethod(FrameDisposalMethod method) { m_disposalMethod = method; }
        void setPremultiplyAlpha(bool premultiplyAlpha) { m_premultiplyAlpha = premultiplyAlpha; }

        inline void setRGBA(int x, int y, unsigned r, unsigned g, unsigned b, unsigned a)
        {
            setRGBA(getAddr(x, y), r, g, b, a);
        }

        void setPixmap(const QPixmap& pixmap);

    private:
        int width() const;
        int height() const;

        inline PixelData* getAddr(int x, int y)
        {

            m_image = m_pixmap.toImage();
            m_pixmap = QPixmap();
            return reinterpret_cast_ptr<QRgb*>(m_image.scanLine(y)) + x;

        }

        inline void setRGBA(PixelData* dest, unsigned r, unsigned g, unsigned b, unsigned a)
        {
            if (m_premultiplyAlpha && !a)
                *dest = 0;
            else {
                if (m_premultiplyAlpha && a < 255) {
                    float alphaPercent = a / 255.0f;
                    r = static_cast<unsigned>(r * alphaPercent);
                    g = static_cast<unsigned>(g * alphaPercent);
                    b = static_cast<unsigned>(b * alphaPercent);
                }
                *dest = (a << 24 | r << 16 | g << 8 | b);
            }
        }

/*#if PLATFORM(SKIA)
        NativeImageSkia m_bitmap;
#elif PLATFORM(QT)*/
        mutable QPixmap m_pixmap;
        mutable QImage m_image;
        bool m_hasAlpha;
        IntSize m_size;
/*#else
        Vector<PixelData> m_bytes;
        IntSize m_size;       // The size of the buffer.  This should be the
                              // same as ImageDecoder::m_size.
        bool m_hasAlpha;      // Whether or not any of the pixels in the buffer
                              // have transparency.
#endif*/

        IntRect m_rect;       // The rect of the original specified frame within
                              // the overall buffer.  This will always just be
                              // the entire buffer except for GIF frames whose
                              // original rect was smaller than the overall
                              // image size.
        FrameStatus m_status; // Whether or not this frame is completely
                              // finished decoding.
        unsigned m_duration;  // The animation delay.
        FrameDisposalMethod m_disposalMethod;
                              // What to do with this frame's data when
                              // initializing the next frame.
        bool m_premultiplyAlpha;
                              // Whether to premultiply alpha into R, G, B
                              // channels; by default it's true.
    };

    // The ImageDecoder class represents a base class for specific image format
    // decoders (e.g., GIF, JPG, PNG, ICO) to derive from.  All decoders decode
    // into RGBA32 format and the base class manages the RGBA32 frame cache.
    //
    // ENABLE(IMAGE_DECODER_DOWN_SAMPLING) allows image decoders to write
    // directly to scaled output buffers by down sampling. Call
    // setMaxNumPixels() to specify the biggest size that decoded images can
    // have. Image decoders will deflate those images that are bigger than
    // m_maxNumPixels. (Not supported by all image decoders yet)
    class ImageDecoder /*: public Noncopyable*/ {
    public:
        ImageDecoder(bool premultiplyAlpha)
            : m_scaled(false)
            , m_premultiplyAlpha(premultiplyAlpha)
            , m_sizeAvailable(false)
            , m_maxNumPixels(-1)
            , m_isAllDataReceived(false)
            , m_failed(false)
        {
        }

        virtual ~ImageDecoder() {}

        // Factory function to create an ImageDecoder.  Ports that subclass
        // ImageDecoder can provide their own implementation of this to avoid
        // needing to write a dedicated setData() implementation.
        static ImageDecoder* create(const SharedBuffer& data, bool premultiplyAlpha);

        // The the filename extension usually associated with an undecoded image
        // of this type.
        virtual String filenameExtension() const = 0;

        bool isAllDataReceived() const { return m_isAllDataReceived; }

        virtual void setData(SharedBuffer* data, bool allDataReceived)
        {
            if (m_failed)
                return;

            //m_data = data;
            m_data->create(data->data(),data->size());

            m_isAllDataReceived = allDataReceived;
        }

        // Whether or not the size information has been decoded yet. This
        // default implementation just returns true if the size has been set and
        // we have not seen a failure. Decoders may want to override this to
        // lazily decode enough of the image to get the size.
        virtual bool isSizeAvailable()
        {
            return !m_failed && m_sizeAvailable;
        }

        // Returns the size of the image.
        virtual IntSize size() const
        {
            return m_size;
        }

        IntSize scaledSize() const
        {
            return m_scaled ? IntSize(m_scaledColumns.size(), m_scaledRows.size()) : size();
        }

        // Returns the size of frame |index|.  This will only differ from size()
        // for formats where different frames are different sizes (namely ICO,
        // where each frame represents a different icon within the master file).
        // Notably, this does not return different sizes for different GIF
        // frames, since while these may be stored as smaller rectangles, during
        // decoding they are composited to create a full-size frame.
        virtual IntSize frameSizeAtIndex(size_t) const
        {
            return size();
        }

        // Called by the image decoders to set their decoded size, this also
        // checks the size for validity. It will return true if the size was
        // set, or false if there is an error. On error, the m_failed flag will
        // be set and the caller should immediately stop decoding.
        virtual bool setSize(unsigned width, unsigned height)
        {
            if (isOverSize(width, height))
                return setFailed();
            m_size = IntSize(width, height);
            m_sizeAvailable = true;
            return true;
        }

        // The total number of frames for the image.  Classes that support
        // multiple frames will scan the image data for the answer if they need
        // to (without necessarily decoding all of the individual frames).
        virtual size_t frameCount() { return 1; }

        // The number of repetitions to perform for an animation loop.
        virtual int repetitionCount() const { return cAnimationNone; }

        // Called to obtain the RGBA32Buffer full of decoded data for rendering.
        // The decoder plugin will decode as much of the frame as it can before
        // handing back the buffer.
        virtual RGBA32Buffer* frameBufferAtIndex(size_t) = 0;

        // Whether or not the underlying image format even supports alpha
        // transparency.
        virtual bool supportsAlpha() const { return true; }

        // Sets the "decode failure" flag.  For caller convenience (since so
        // many callers want to return false after calling this), returns false
        // to enable easy tailcalling.  Subclasses may override this to also
        // clean up any local data.
        virtual bool setFailed()
        {
            m_failed = true;
            return false;
        }

        bool failed() const { return m_failed; }

        // Wipe out frames in the frame buffer cache before |clearBeforeFrame|,
        // assuming this can be done without breaking decoding.  Different
        // decoders place different restrictions on what frames are safe to
        // destroy, so this is left to them to implement.
        // For convenience's sake, we provide a default (empty) implementation,
        // since in practice only GIFs will ever use this.
        virtual void clearFrameBufferCache(size_t clearBeforeFrame) { }

/*#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
        void setMaxNumPixels(int m) { m_maxNumPixels = m; }
#endif*/

    protected:
        void prepareScaleDataIfNecessary();
        int upperBoundScaledX(int origX, int searchStart = 0);
        int lowerBoundScaledX(int origX, int searchStart = 0);
        int upperBoundScaledY(int origY, int searchStart = 0);
        int lowerBoundScaledY(int origY, int searchStart = 0);
        int scaledY(int origY, int searchStart = 0);

        RefPtr<SharedBuffer> m_data; // The encoded data.
        QVector<RGBA32Buffer> m_frameBufferCache;
        bool m_scaled;
        QVector<int> m_scaledColumns;
        QVector<int> m_scaledRows;
        bool m_premultiplyAlpha;

    private:
        // Some code paths compute the size of the image as "width * height * 4"
        // and return it as a (signed) int.  Avoid overflow.
        static bool isOverSize(unsigned width, unsigned height)
        {
            // width * height must not exceed (2 ^ 29) - 1, so that we don't
            // overflow when we multiply by 4.
            unsigned long long total_size = static_cast<unsigned long long>(width)
                                          * static_cast<unsigned long long>(height);
            return total_size > ((1 << 29) - 1);
        }

        IntSize m_size;
        bool m_sizeAvailable;
        int m_maxNumPixels;
        bool m_isAllDataReceived;
        bool m_failed;
    };

