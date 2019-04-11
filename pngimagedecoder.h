#include "common.h"
#include "imagedecoder.h"
//#include <wtf/OwnPtr.h>


    class PNGImageReader;

    // This class decodes the PNG image format.
    class PNGImageDecoder : public ImageDecoder {
    public:
        PNGImageDecoder(bool premultiplyAlpha);
        virtual ~PNGImageDecoder();

        // ImageDecoder
        virtual String filenameExtension() const { return "png"; }
        virtual bool isSizeAvailable();
        virtual bool setSize(unsigned width, unsigned height);
        virtual RGBA32Buffer* frameBufferAtIndex(size_t index);
        // CAUTION: setFailed() deletes |m_reader|.  Be careful to avoid
        // accessing deleted memory, especially when calling this from inside
        // PNGImageReader!
        virtual bool setFailed();

        // Callbacks from libpng
        void headerAvailable();
        void rowAvailable(unsigned char* rowBuffer, unsigned rowIndex, int interlacePass);
        void pngComplete();

        bool isComplete() const
        {
            return !m_frameBufferCache.isEmpty() && (m_frameBufferCache.first().status() == RGBA32Buffer::FrameComplete);
        }

    private:
        // Decodes the image.  If |onlySize| is true, stops decoding after
        // calculating the image size.  If decoding fails but there is no more
        // data coming, sets the "decode failure" flag.
        void decode(bool onlySize);

        OwnPtr<PNGImageReader> m_reader;
        bool m_doNothingOnFailure;
    };
