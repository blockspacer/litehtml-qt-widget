#pragma once

#include "common.h"

//#include <wtf/Forward.h>
//#include <wtf/Noncopyable.h>
//#include <wtf/Vector.h>


#include <qglobal.h>
QT_BEGIN_NAMESPACE
class QPixmap;
QT_END_NAMESPACE

class IntPoint;
class IntSize;
class SharedBuffer;

class ImageDecoderQt;
typedef ImageDecoderQt* NativeImageSourcePtr;
typedef QPixmap* NativeImagePtr;

// Right now GIFs are the only recognized image format that supports animation.
// The animation system and the constants below are designed with this in mind.
// GIFs have an optional 16-bit unsigned loop count that describes how an
// animated GIF should be cycled.  If the loop count is absent, the animation
// cycles once; if it is 0, the animation cycles infinitely; otherwise the
// animation plays n + 1 cycles (where n is the specified loop count).  If the
// GIF decoder defaults to cAnimationLoopOnce in the absence of any loop count
// and translates an explicit "0" loop count to cAnimationLoopInfinite, then we
// get a couple of nice side effects:
//   * By making cAnimationLoopOnce be 0, we allow the animation cycling code in
//     BitmapImage.cpp to avoid special-casing it, and simply treat all
//     non-negative loop counts identically.
//   * By making the other two constants negative, we avoid conflicts with any
//     real loop count values.
const int cAnimationLoopOnce = 0;
const int cAnimationLoopInfinite = -1;
const int cAnimationNone = -2;

class ImageSource /*: public Noncopyable*/ {
public:
    ImageSource(bool premultiplyAlpha = true);
    ~ImageSource();

    // Tells the ImageSource that the Image no longer cares about decoded frame
    // data -- at all (if |destroyAll| is true), or before frame
    // |clearBeforeFrame| (if |destroyAll| is false).  The ImageSource should
    // delete cached decoded data for these frames where possible to keep memory
    // usage low.  When |destroyAll| is true, the ImageSource should also reset
    // any local state so that decoding can begin again.
    //
    // Implementations that delete less than what's specified above waste
    // memory.  Implementations that delete more may burn CPU re-decoding frames
    // that could otherwise have been cached, or encounter errors if they're
    // asked to decode frames they can't decode due to the loss of previous
    // decoded frames.
    //
    // Callers should not call clear(false, n) and subsequently call
    // createFrameAtIndex(m) with m < n, unless they first call clear(true).
    // This ensures that stateful ImageSources/decoders will work properly.
    //
    // The |data| and |allDataReceived| parameters should be supplied by callers
    // who set |destroyAll| to true if they wish to be able to continue using
    // the ImageSource.  This way implementations which choose to destroy their
    // decoders in some cases can reconstruct them correctly.
    void clear(bool destroyAll,
               size_t clearBeforeFrame = 0,
               SharedBuffer* data = nullptr,
               bool allDataReceived = false);

    bool initialized() const;

    void setData(SharedBuffer* data, bool allDataReceived);
    String filenameExtension() const;

    bool isSizeAvailable();
    IntSize size() const;
    IntSize frameSizeAtIndex(size_t) const;
    bool getHotSpot(IntPoint&) const;

    int repetitionCount();

    size_t frameCount() const;

    // Callers should not call this after calling clear() with a higher index;
    // see comments on clear() above.
    NativeImagePtr createFrameAtIndex(size_t);

    float frameDurationAtIndex(size_t);
    bool frameHasAlphaAtIndex(size_t); // Whether or not the frame actually used any alpha.
    bool frameIsCompleteAtIndex(size_t); // Whether or not the frame is completely decoded.

/*#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    static unsigned maxPixelsPerDecodedImage() { return s_maxPixelsPerDecodedImage; }
    static void setMaxPixelsPerDecodedImage(unsigned maxPixels) { s_maxPixelsPerDecodedImage = maxPixels; }
#endif*/

private:
    NativeImageSourcePtr m_decoder;
    bool m_premultiplyAlpha;
/*#if ENABLE(IMAGE_DECODER_DOWN_SAMPLING)
    static unsigned s_maxPixelsPerDecodedImage;
#endif*/
};
