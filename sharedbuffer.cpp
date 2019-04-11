#include "common.h"
#include "sharedbuffer.h"
#include "purgeablebuffer.h"

//#include "PurgeableBuffer.h"
//#include <wtf/PassOwnPtr.h>

using namespace std;

static const unsigned segmentSize = 0x1000;
static const unsigned segmentPositionMask = 0x0FFF;

static inline unsigned segmentIndex(unsigned position)
{
    return position / segmentSize;
}

static inline unsigned offsetInSegment(unsigned position)
{
    return position & segmentPositionMask;
}

static inline char* allocateSegment()
{
    return static_cast<char*>(malloc(segmentSize));
}

static inline void freeSegment(char* p)
{
    free(p);
}

SharedBuffer::SharedBuffer()
    : m_size(0)
{
}

SharedBuffer::SharedBuffer(const char* data, int size)
    : m_size(0)
{
    append(data, size);
}

SharedBuffer::SharedBuffer(const unsigned char* data, int size)
    : m_size(0)
{
    append(reinterpret_cast<const char*>(data), size);
}

SharedBuffer::~SharedBuffer()
{
    clear();
}

PassRefPtr<SharedBuffer> SharedBuffer::adoptVector(QVector<char>& vector)
{
    RefPtr<SharedBuffer> buffer = create();
    buffer->m_buffer.swap(vector);
    buffer->m_size = buffer->m_buffer.size();
    return buffer;//buffer.release();
}

PassRefPtr<SharedBuffer> SharedBuffer::adoptPurgeableBuffer(PassOwnPtr<PurgeableBuffer> purgeableBuffer)
{
    ASSERT(!purgeableBuffer->isPurgeable());
    RefPtr<SharedBuffer> buffer = create();
    //buffer->m_purgeableBuffer = purgeableBuffer;
    buffer->m_purgeableBuffer.swap(  purgeableBuffer );
    return buffer;//buffer.release();
}

unsigned SharedBuffer::size() const
{
    if (hasPlatformData())
        return platformDataSize();

    if (m_purgeableBuffer)
        return m_purgeableBuffer->size();

    return m_size;
}

const char* SharedBuffer::data() const
{
    if (hasPlatformData())
        return platformData();

    if (m_purgeableBuffer)
        return m_purgeableBuffer->data();

    return buffer().data();
}

void SharedBuffer::append(const char* data, unsigned length)
{
    ASSERT(!m_purgeableBuffer);

    maybeTransferPlatformData();

    unsigned positionInSegment = offsetInSegment(m_size - m_buffer.size());
    m_size += length;

    if (m_size <= segmentSize) {
        // No need to use segments for small resource data
        //m_buffer.append(data, length);

        //m_buffer.resize(length);
        //memcpy(&m_buffer[0], data,length);
        std::copy(data, data + length, std::back_inserter(m_buffer));
        return;
    }

    char* segment;
    if (!positionInSegment) {
        segment = allocateSegment();
        m_segments.append(segment);
    } else
        segment = m_segments.last() + positionInSegment;

    unsigned segmentFreeSpace = segmentSize - positionInSegment;
    unsigned bytesToCopy = min(length, segmentFreeSpace);

    for (;;) {
        memcpy(segment, data, bytesToCopy);
        if (static_cast<unsigned>(length) == bytesToCopy)
            break;

        length -= bytesToCopy;
        data += bytesToCopy;
        segment = allocateSegment();
        m_segments.append(segment);
        bytesToCopy = min(length, segmentSize);
    }
}

void SharedBuffer::clear()
{
    clearPlatformData();

    for (unsigned i = 0; i < m_segments.size(); ++i)
        freeSegment(m_segments[i]);

    m_segments.clear();
    m_size = 0;

    m_buffer.clear();
    //m_purgeableBuffer.clear();
    m_purgeableBuffer.reset();
}

PassRefPtr<SharedBuffer> SharedBuffer::copy() const
{
    RefPtr<SharedBuffer> clone(adoptRef<SharedBuffer>( ));
    if (m_purgeableBuffer || hasPlatformData()) {
        clone->append(data(), size());
        return clone;
    }

    clone->m_size = m_size;
    clone->m_buffer.reserve(m_size);//reserveCapacity(m_size);
    //clone->m_buffer = m_buffer;//append(m_buffer.data(), m_buffer.size());
    std::copy(m_buffer.data(), m_buffer.data() + m_buffer.size(), std::back_inserter(m_buffer));

    // TODO: do we need it >>>>>????
    ///
    //for (unsigned i = 0; i < m_segments.size(); ++i)
    //    clone->m_buffer.push_back(m_segments[i]);//.append(m_segments[i], segmentSize);

    return clone;
}

PassOwnPtr<PurgeableBuffer> SharedBuffer::releasePurgeableBuffer()
{
    //ASSERT(hasOneRef());
    //return m_purgeableBuffer.release();
    return std::move(m_purgeableBuffer);
}

const QVector<char>& SharedBuffer::buffer() const
{
    unsigned bufferSize = m_buffer.size();
    if (m_size > bufferSize) {
        m_buffer.resize(m_size);
        char* destination = m_buffer.data() + bufferSize;
        unsigned bytesLeft = m_size - bufferSize;
        for (unsigned i = 0; i < m_segments.size(); ++i) {
            unsigned bytesToCopy = min(bytesLeft, segmentSize);
            memcpy(destination, m_segments[i], bytesToCopy);
            destination += bytesToCopy;
            bytesLeft -= bytesToCopy;
            freeSegment(m_segments[i]);
        }
        m_segments.clear();
    }
    return m_buffer;
}

unsigned SharedBuffer::getSomeData(const char*& someData, unsigned position) const
{
    if (hasPlatformData() || m_purgeableBuffer) {
        someData = data() + position;
        return size() - position;
    }

    if (position >= m_size) {
        someData = 0;
        return 0;
    }

    unsigned consecutiveSize = m_buffer.size();
    if (position < consecutiveSize) {
        someData = m_buffer.data() + position;
        return consecutiveSize - position;
    }

    position -= consecutiveSize;
    unsigned segmentedSize = m_size - consecutiveSize;
    unsigned segments = m_segments.size();
    unsigned segment = segmentIndex(position);
    ASSERT(segment < segments);

    unsigned positionInSegment = offsetInSegment(position);
    someData = m_segments[segment] + positionInSegment;
    return segment == segments - 1 ? segmentedSize - position : segmentSize - positionInSegment;
}

//#if !PLATFORM(CF) || PLATFORM(QT) || PLATFORM(CLUTTER)

inline void SharedBuffer::clearPlatformData()
{
}

inline void SharedBuffer::maybeTransferPlatformData()
{
}

inline bool SharedBuffer::hasPlatformData() const
{
    return false;
}

inline const char* SharedBuffer::platformData() const
{
    //ASSERT_NOT_REACHED();

    return 0;
}

inline unsigned SharedBuffer::platformDataSize() const
{
    //ASSERT_NOT_REACHED();

    return 0;
}

//#endif
