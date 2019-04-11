#pragma once

#include "common.h"
#include "image.h"
#include "imagesource.h"

    class StillImage : public Image {
    public:
        static PassRefPtr<StillImage> create(const QPixmap& pixmap)
        {
            return adoptRef<StillImage>(pixmap);
        }

        static PassRefPtr<StillImage> createForRendering(const QPixmap* pixmap)
        {
            return adoptRef<StillImage>(pixmap);
        }

        // FIXME: StillImages are underreporting decoded sizes and will be unable
        // to prune because these functions are not implemented yet.
        virtual void destroyDecodedData(bool destroyAll = true) { Q_UNUSED(destroyAll); }
        virtual unsigned decodedSize() const { return 0; }

        virtual IntSize size() const;
        virtual NativeImagePtr nativeImageForCurrentFrame();
        virtual void draw(GraphicsContext*, const FloatRect& dstRect, const FloatRect& srcRect, ColorSpace styleColorSpace, CompositeOperator);

    //private:
    public:
        StillImage(const QPixmap& pixmap);
        StillImage(const QPixmap* pixmap);
        ~StillImage();

    private:
        const QPixmap* m_pixmap;
        bool m_ownsPixmap;
    };
