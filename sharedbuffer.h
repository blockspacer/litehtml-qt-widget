#pragma once

#include "common.h"
/*
#include "PlatformString.h"
#include <wtf/Forward.h>
#include <wtf/OwnPtr.h>
#include <wtf/RefCounted.h>
#include <wtf/Vector.h>*/

/*#if PLATFORM(CF)
#include <wtf/RetainPtr.h>
#endif

#if PLATFORM(MAC)
#ifdef __OBJC__
@class NSData;
#else
class NSData;
#endif

#endif*/

class PurgeableBuffer;

class SharedBuffer /*: public RefCounted<SharedBuffer>*/ {
public:
    static PassRefPtr<SharedBuffer> create() { return adoptRef<SharedBuffer>( ); }
    static PassRefPtr<SharedBuffer> create(const char* c, int i) { return adoptRef<SharedBuffer>(c, i); }
    static PassRefPtr<SharedBuffer> create(const unsigned char* c, int i) { return adoptRef<SharedBuffer>(c, i); }

    static PassRefPtr<SharedBuffer> createWithContentsOfFile(const String& filePath);

    static PassRefPtr<SharedBuffer> adoptVector(QVector<char>& vector);

    // The buffer must be in non-purgeable state before adopted to a SharedBuffer.
    // It will stay that way until released.
    static PassRefPtr<SharedBuffer> adoptPurgeableBuffer(PassOwnPtr<PurgeableBuffer>);

    ~SharedBuffer();

/*#if PLATFORM(MAC)
    NSData *createNSData();
    static PassRefPtr<SharedBuffer> wrapNSData(NSData *data);
#endif
#if PLATFORM(CF)
    CFDataRef createCFData();
    static PassRefPtr<SharedBuffer> wrapCFData(CFDataRef);
#endif*/

    // Calling this function will force internal segmented buffers
    // to be merged into a flat buffer. Use getSomeData() whenever possible
    // for better performance.
    const char* data() const;

    unsigned size() const;

    // Calling this function will force internal segmented buffers
    // to be merged into a flat buffer. Use getSomeData() whenever possible
    // for better performance.
    const QVector<char>& buffer() const;

    bool isEmpty() const { return !size(); }

    void append(const char*, unsigned);
    void clear();
    const char* platformData() const;
    unsigned platformDataSize() const;

    PassRefPtr<SharedBuffer> copy() const;

    bool hasPurgeableBuffer() const { return m_purgeableBuffer.get(); }

    // Ensure this buffer has no other clients before calling this.
    PassOwnPtr<PurgeableBuffer> releasePurgeableBuffer();

    // Return the number of consecutive bytes after "position". "data"
    // points to the first byte.
    // Return 0 when no more data left.
    // When extracting all data with getSomeData(), the caller should
    // repeat calling it until it returns 0.
    // Usage:
    //      const char* segment;
    //      unsigned pos = 0;
    //      while (unsigned length = sharedBuffer->getSomeData(segment, pos)) {
    //          // Use the data. for example: decoder->decode(segment, length);
    //          pos += length;
    //      }
    unsigned getSomeData(const char*& data, unsigned position = 0) const;

//private:
public:
    SharedBuffer();
    SharedBuffer(const char*, int);
    SharedBuffer(const unsigned char*, int);

private:
    void clearPlatformData();
    void maybeTransferPlatformData();
    bool hasPlatformData() const;

    unsigned m_size;
    mutable QVector<char> m_buffer;
    mutable QVector<char*> m_segments;
    OwnPtr<PurgeableBuffer> m_purgeableBuffer;
/*#if PLATFORM(CF)
    SharedBuffer(CFDataRef);
    RetainPtr<CFDataRef> m_cfData;
#endif*/
};
