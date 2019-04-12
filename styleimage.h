#pragma once

#include "common.h"
#include "intsize.h"

/*#include "CSSValue.h"
#include "IntSize.h"
#include <wtf/PassRefPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/RefPtr.h>*/

// TODO
//class CSSValue;

class Image;
class RenderObject;

typedef void* WrappedImagePtr;

class StyleImage /*: public RefCounted<StyleImage>*/ {
public:
    virtual ~StyleImage() { }

    bool operator==(const StyleImage& other) const
    {
        return data() == other.data();
    }

    //virtual PassRefPtr<CSSValue> cssValue() const = 0;

    virtual bool canRender(float /*multiplier*/) const { return true; }
    virtual bool isLoaded() const { return true; }
    virtual bool errorOccurred() const { return false; }
    virtual IntSize imageSize(const RenderObject*, float multiplier) const = 0;
    virtual bool imageHasRelativeWidth() const = 0;
    virtual bool imageHasRelativeHeight() const = 0;
    virtual bool usesImageContainerSize() const = 0;
    virtual void setImageContainerSize(const IntSize&) = 0;
    virtual void addClient(RenderObject*) = 0;
    virtual void removeClient(RenderObject*) = 0;
    virtual Image* image(RenderObject*, const IntSize&) const = 0;
    virtual WrappedImagePtr data() const = 0;

    virtual bool isCachedImage() const { return false; }
    virtual bool isPendingImage() const { return false; }
    virtual bool isGeneratedImage() const { return false; }

    static  bool imagesEquivalent(StyleImage* image1, StyleImage* image2)
    {
        if (image1 != image2) {
            if (!image1 || !image2)
                return false;
            return *image1 == *image2;
        }
        return true;
    }

protected:
    StyleImage() { }
};
