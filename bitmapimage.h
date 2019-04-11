#pragma once

#include "common.h"
#include "image.h"
#include "color.h"
#include "intsize.h"
#include "imageobserver.h"

struct FrameData;

// This complicated-looking declaration tells the FrameData Vector that it should copy without
// invoking our constructor or destructor. This allows us to have a vector even for a struct
// that's not copyable.
/*namespace WTF {
    template<> struct VectorTraits<WebCore::FrameData> : public SimpleClassVectorTraits {};
}*/

    // TODO
//template <typename T> class Timer;

// ================================================
// FrameData Class
// ================================================

struct FrameData /*: Noncopyable*/ {
    FrameData()
        : m_frame(0)
        , m_haveMetadata(false)
        , m_isComplete(false)
        , m_duration(0)
        , m_hasAlpha(true)
    {
    }

    ~FrameData()
    {
        clear(true);
    }

    // Clear the cached image data on the frame, and (optionally) the metadata.
    // Returns whether there was cached image data to clear.
    bool clear(bool clearMetadata);

    NativeImagePtr m_frame;
    bool m_haveMetadata;
    bool m_isComplete;
    float m_duration;
    bool m_hasAlpha;
};

// =================================================
// BitmapImage Class
// =================================================

class BitmapImage : public Image {
    friend class GeneratedImage;
    friend class GraphicsContext;
public:
    static PassRefPtr<BitmapImage> create(NativeImagePtr nativeImage, ImageObserver* observer = 0)
    {
        return adoptRef<BitmapImage>(nativeImage, observer);
    }
    static PassRefPtr<BitmapImage> create(ImageObserver* observer = 0)
    {
        return adoptRef<BitmapImage>(observer);
    }
    ~BitmapImage();

    virtual bool isBitmapImage() const { return true; }

    virtual bool hasSingleSecurityOrigin() const { return true; }

    virtual IntSize size() const;
    IntSize currentFrameSize() const;
    virtual bool getHotSpot(IntPoint&) const;

    virtual bool dataChanged(bool allDataReceived);
    virtual String filenameExtension() const;

    // It may look unusual that there is no start animation call as public API.  This is because
    // we start and stop animating lazily.  Animation begins whenever someone draws the image.  It will
    // automatically pause once all observers no longer want to render the image anywhere.
    virtual void stopAnimation();
    virtual void resetAnimation();

    virtual unsigned decodedSize() const { return m_decodedSize; }

    virtual NativeImagePtr nativeImageForCurrentFrame() { return frameAtIndex(currentFrame()); }

//protected:
public:
    enum RepetitionCountStatus {
      Unknown,    // We haven't checked the source's repetition count.
      Uncertain,  // We have a repetition count, but it might be wrong (some GIFs have a count after the image data, and will report "loop once" until all data has been decoded).
      Certain,    // The repetition count is known to be correct.
    };

    BitmapImage(NativeImagePtr, ImageObserver* = 0);
    BitmapImage(ImageObserver* = 0);

protected:
    virtual void draw(GraphicsContext*, const FloatRect& dstRect, const FloatRect& srcRect, ColorSpace styleColorSpace, CompositeOperator);


    size_t currentFrame() const { return m_currentFrame; }
    size_t frameCount();
    NativeImagePtr frameAtIndex(size_t);
    bool frameIsCompleteAtIndex(size_t);
    float frameDurationAtIndex(size_t);
    bool frameHasAlphaAtIndex(size_t);

    // Decodes and caches a frame. Never accessed except internally.
    void cacheFrame(size_t index);

    // Called to invalidate cached data.  When |destroyAll| is true, we wipe out
    // the entire frame buffer cache and tell the image source to destroy
    // everything; this is used when e.g. we want to free some room in the image
    // cache.  If |destroyAll| is false, we only delete frames up to the current
    // one; this is used while animating large images to keep memory footprint
    // low without redecoding the whole image on every frame.
    virtual void destroyDecodedData(bool destroyAll = true);

    // If the image is large enough, calls destroyDecodedData() and passes
    // |destroyAll| along.
    void destroyDecodedDataIfNecessary(bool destroyAll);

    // Generally called by destroyDecodedData(), destroys whole-image metadata
    // and notifies observers that the memory footprint has (hopefully)
    // decreased by |framesCleared| times the size (in bytes) of a frame.
    void destroyMetadataAndNotify(int framesCleared);

    // Whether or not size is available yet.
    bool isSizeAvailable();

    // Animation.
    int repetitionCount(bool imageKnownToBeComplete);  // |imageKnownToBeComplete| should be set if the caller knows the entire image has been decoded.
    bool shouldAnimate();
    virtual void startAnimation(bool catchUpIfNecessary = true);

    // TODO
    //void advanceAnimation(Timer<BitmapImage>*);

    // Function that does the real work of advancing the animation.  When
    // skippingFrames is true, we're in the middle of a loop trying to skip over
    // a bunch of animation frames, so we should not do things like decode each
    // one or notify our observers.
    // Returns whether the animation was advanced.
    bool internalAdvanceAnimation(bool skippingFrames);

    // Handle platform-specific data
    void initPlatformData();
    void invalidatePlatformData();

    // Checks to see if the image is a 1x1 solid color.  We optimize these images and just do a fill rect instead.
    // This check should happen regardless whether m_checkedForSolidColor is already set, as the frame may have
    // changed.
    void checkForSolidColor();

    virtual bool mayFillWithSolidColor()
    {
        if (!m_checkedForSolidColor && frameCount() > 0) {
            checkForSolidColor();
            // WINCE PORT: checkForSolidColor() doesn't set m_checkedForSolidColor until
            // it gets enough information to make final decision.

            ASSERT(m_checkedForSolidColor);

        }
        return m_isSolidColor && m_currentFrame == 0;
    }
    virtual Color solidColor() const { return m_solidColor; }

    ImageSource m_source;
    mutable IntSize m_size; // The size to use for the overall image (will just be the size of the first image).

    size_t m_currentFrame; // The index of the current frame of animation.
    QVector<FrameData> m_frames; // An array of the cached frames of the animation. We have to ref frames to pin them in the cache.

    // TODO
    //Timer<BitmapImage>* m_frameTimer;

    int m_repetitionCount; // How many total animation loops we should do.  This will be cAnimationNone if this image type is incapable of animation.
    RepetitionCountStatus m_repetitionCountStatus;
    int m_repetitionsComplete;  // How many repetitions we've finished.
    double m_desiredFrameStartTime;  // The system time at which we hope to see the next call to startAnimation().

    Color m_solidColor;  // If we're a 1x1 solid color, this is the color to use to fill.
    bool m_isSolidColor;  // Whether or not we are a 1x1 solid image.
    bool m_checkedForSolidColor; // Whether we've checked the frame for solid color.

    bool m_animationFinished;  // Whether or not we've completed the entire animation.

    bool m_allDataReceived;  // Whether or not we've received all our data.

    mutable bool m_haveSize; // Whether or not our |m_size| member variable has the final overall image size yet.
    bool m_sizeAvailable; // Whether or not we can obtain the size of the first image frame yet from ImageIO.
    mutable bool m_hasUniformFrameSize;

    unsigned m_decodedSize; // The current size of all decoded frames.

    mutable bool m_haveFrameCount;
    size_t m_frameCount;
};
