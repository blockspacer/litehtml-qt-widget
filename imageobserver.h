#pragma once

class Image;
class IntRect;

// Interface for notification about changes to an image, including decoding,
// drawing, and animating.
class ImageObserver {
protected:
    virtual ~ImageObserver() {}
public:
    virtual void decodedSizeChanged(const Image*, int delta) = 0;
    virtual void didDraw(const Image*) = 0;

    virtual bool shouldPauseAnimation(const Image*) = 0;
    virtual void animationAdvanced(const Image*) = 0;

    virtual void changedInRect(const Image*, const IntRect&) = 0;
};
