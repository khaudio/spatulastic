#include "ringbuffer.h"

using namespace Buffer;

template <typename T>
constexpr T RingBuffer<T>::_zero = get_zero<T>();

Ring::Ring() :
ringLength(0)
{
}

Ring::~Ring()
{
}

template <typename T>
RingBuffer<T>::RingBuffer() :
Ring(),
bufferLength(0)
{
}

template <typename T>
RingBuffer<T>::RingBuffer(int bufferSize, uint8_t ringSize) :
Ring(),
_buffered(0),
_samplesWritten(0),
_samplesRemaining(bufferSize),
_samplesProcessed(0),
bufferLength(bufferSize),
bytesPerSample(sizeof(T)),
bytesPerBuffer(bufferSize * sizeof(T)),
readIndex(0),
writeIndex(1),
processingIndex(0)
{
    #if _DEBUG
    if (ringSize < 2) throw RING_SIZE_TOO_SHORT;
    #endif

    this->ringLength = ringSize;
    this->totalRingSampleLength = this->ringLength * this->bufferLength;
    this->_totalWritableLength = this->totalRingSampleLength - this->bufferLength;
    this->ring.reserve(this->ringLength);
    for (int i(0); i < this->ringLength; ++i)
    {
        this->ring.emplace_back(std::vector<T>());
        this->ring[i].reserve(this->bufferLength);
        for (uint32_t j(0); j < this->bufferLength; ++j)
        {
            this->ring[i].emplace_back(_zero);
        }
    }
    for (int i(0); i < this->ringLength; ++i)
    {
        this->bufferProcessedState.emplace(std::make_pair(i, false));
    }
}

template <typename T>
RingBuffer<T>::~RingBuffer()
{
}

template <typename T>
inline bool RingBuffer<T>::_size_is_set()
{
    return (this->bufferLength && this->ringLength);
}

template <typename T>
void RingBuffer<T>::set_size(int bufferSize, uint8_t ringSize)
{
    this->bufferLength = bufferSize;
    this->ringLength = ringSize;
    this->totalRingSampleLength = this->ringLength * this->bufferLength;
    this->_totalWritableLength = this->totalRingSampleLength - this->bufferLength;
    this->bytesPerBuffer = this->bufferLength * sizeof(T);
    this->_samplesRemaining = this->bufferLength;
    this->_samplesWritten = 0;
    this->_buffered = 0;
    this->ring = std::vector<std::vector<T>>();
    this->ring.reserve(this->ringLength);
    for (int i(0); i < this->ringLength; ++i)
    {
        this->ring.emplace_back(std::vector<T>());
        this->ring[i].reserve(this->bufferLength);
        for (uint32_t j(0); j < this->bufferLength; ++j)
        {
            this->ring[i].emplace_back(_zero);
        }
    }
    this->readIndex = 0;
    this->writeIndex = 1;
    this->processingIndex = 0;
}

template <typename T>
size_t RingBuffer<T>::size()
{
    /* Total number of samples that can fit in all rings.
    Returns samples, not bytes. */
    return this->totalRingSampleLength;
}

template <typename T>
void RingBuffer<T>::zero_fill()
{
    /* Fill all the buffers with zero */
    for (int i(0); i < this->bufferLength; ++i)
    {
        this->ring[0][i] = this->_zero;
    }
    for (int i(1); i < this->ringLength; ++i)
    {
        std::copy(
                this->ring[0].begin(),
                this->ring[0].end(),
                this->ring[i].begin()
            );
    }
}

template <typename T>
void RingBuffer<T>::reset(bool zeroFill)
{
    if (zeroFill) zero_fill();
    this->_samplesRemaining = this->bufferLength;
    this->_samplesWritten = 0;
    this->_buffered = 0;
    this->readIndex = 0;
    this->writeIndex = 1;
    this->processingIndex = 0;
    for (int i(0); i < this->ringLength; ++i)
    {
        this->bufferProcessedState[i] = false;
    }
}

template <typename T>
int RingBuffer<T>::buffered()
{
    /* Total number of unread samples buffered,
    excluding the current write buffer */
    return this->_buffered;
}

template <typename T>
int RingBuffer<T>::available()
{
    /* Total number of samples unbuffered,
    excluding current read buffer */
    return this->_totalWritableLength - this->_buffered;
}

template <typename T>
int RingBuffer<T>::processed()
{
    /* Total number of samples processed */
    return this->_samplesProcessed;
}

template <typename T>
int RingBuffer<T>::buffers_buffered()
{
    /* Total number of unread readable buffers */
    return (this->_buffered / this->bufferLength);
}

template <typename T>
int RingBuffer<T>::buffers_available()
{
    /* Total number of writable buffers */
    return (available() / this->bufferLength);
}

template <typename T>
int RingBuffer<T>::buffers_processed()
{
    /* Total number of unread readable buffers */
    return (this->_samplesProcessed / this->bufferLength);
}

template <typename T>
bool RingBuffer<T>::is_writable()
{
    /* Checks bounds to prevent buffer collisions */
    #ifdef _DEBUG
    if (!_size_is_set()) throw BUFFER_NOT_INITIALIZED;
    #endif
    
    return this->readIndex != this->writeIndex;
}

template <typename T>
inline void RingBuffer<T>::rotate_read_index()
{
    /* Rotate read index wihtout changing sample counters */
    if (++this->readIndex >= this->ringLength)
    {
        this->readIndex = 0;
    }
}

template <typename T>
inline void RingBuffer<T>::rotate_write_index()
{
    /* Rotate write index wihtout changing sample counters */
    if (++this->writeIndex >= this->ringLength)
    {
        this->writeIndex = 0;
    }
}

template <typename T>
inline void RingBuffer<T>::rotate_processing_index()
{
    /* Rotate processing index wihtout changing sample counters */
    if (++this->processingIndex >= this->ringLength)
    {
        this->processingIndex = 0;
    }
}

template <typename T>
void RingBuffer<T>::rotate_read_buffer(bool force)
{
    /* Rotates read buffer and forces write buffer forward if overrun */
    rotate_read_index();
    this->_buffered -= this->bufferLength;
    this->_buffered = (this->_buffered < 0) ? 0 : this->_buffered;
    this->_samplesProcessed -= this->bufferLength;
    this->_samplesProcessed = (
            (this->_samplesProcessed < 0)
            ? 0 : this->_samplesProcessed
        );
    if (force && !is_writable())
    {
        rotate_write_buffer();
    }
}

template <typename T>
void RingBuffer<T>::rotate_write_buffer(bool force)
{
    /* Rotates write buffer and forces read buffer forward if overrun */
    rotate_write_index();
    this->_samplesWritten = 0;
    this->_samplesRemaining = this->bufferLength;
    this->_buffered += this->bufferLength;
    _set_buffer_processed(this->writeIndex, false);
    if (force && !is_writable())
    {
        rotate_read_buffer();
    }
}

template <typename T>
void RingBuffer<T>::rotate_processing_buffer()
{
    /* Marks buffer as processed and advances the processing index */
    _set_buffer_processed(this->processingIndex, true);
    this->_samplesProcessed += this->bufferLength;
    rotate_processing_index();
}

template <typename T>
void RingBuffer<T>::rotate_partial_read(unsigned int length, bool force)
{
    /* Rotates read buffer after reading only a specified
    number of samples instead of the entire buffer */
    #if _DEBUG
    if (length > this->bufferLength)
    {
        throw std::out_of_range("Length must be <= buffer length");
    }
    #endif

    rotate_read_index();
    this->_buffered -= length;
    this->_buffered = (this->_buffered < 0) ? 0 : this->_buffered;
    this->_samplesProcessed -= length;
    this->_samplesProcessed = (
            (this->_samplesProcessed < 0)
            ? 0 : this->_samplesProcessed
        );
    if (force && !is_writable())
    {
        rotate_write_buffer();
    }
}

template <typename T>
void RingBuffer<T>::rotate_partial_write(unsigned int length, bool force)
{
    /* Rotates write buffer after writing only a specified
    number of samples instead of the entire buffer */
    #if _DEBUG
    if (length > this->bufferLength)
    {
        throw std::out_of_range("Length must be <= buffer length");
    }
    #endif

    rotate_write_index();
    this->_samplesWritten = 0;
    this->_samplesRemaining = this->bufferLength;
    this->_buffered += length;
    this->_buffered = (
            (this->_buffered > this->_totalWritableLength)
            ? this->_totalWritableLength : this->_buffered
        );
    _set_buffer_processed(this->writeIndex, false);
    if (force && !is_writable())
    {
        rotate_read_buffer();
    }
}

template <typename T>
void RingBuffer<T>::rotate_partial_processing(unsigned int length)
{
    /* Rotates processing buffer after processing only
    a specified number of samples instead of the entire buffer */
    #if _DEBUG
    if (length > this->bufferLength)
    {
        throw std::out_of_range("Length must be <= buffer length");
    }
    #endif

    _set_buffer_processed(this->processingIndex, true);
    this->_samplesProcessed += length;
    this->_samplesProcessed = (
        (this->_samplesProcessed > this->_totalWritableLength)
        ? this->_totalWritableLength : this->_samplesProcessed
    );
    rotate_processing_index();
}


template <typename T>
inline uint8_t RingBuffer<T>::get_ring_index(std::vector<T>* bufferPtr)
{
    /* Returns ring index for buffer at pointer */
    #ifdef _DEBUG
    if (!_size_is_set()) throw BUFFER_NOT_INITIALIZED;
    #endif

    for (uint8_t i(0); i < this->ringLength; ++i)
    {
        if (&(this->ring[i]) == bufferPtr)
        {
            return i;
        }
    }

    #if _DEBUG
    throw std::out_of_range("Buffer not found");
    #endif
}

template <typename T>
inline uint8_t RingBuffer<T>::get_ring_index(uint8_t* bufferPtr)
{
    /* Returns ring index for buffer beginning at pointer */
    #ifdef _DEBUG
    if (!_size_is_set()) throw BUFFER_NOT_INITIALIZED;
    #endif

    for (uint8_t i(0); i < this->ringLength; ++i)
    {
        if (reinterpret_cast<uint8_t*>(&(this->ring[i][0])) == bufferPtr)
        {
            return i;
        }
    }

    #if _DEBUG
    throw std::out_of_range("Buffer not found");
    #endif
}

template <typename T>
inline std::vector<T>* RingBuffer<T>::get_read_buffer()
{
    /* Returns pointer to current read buffer */
    return &(this->ring[this->readIndex]);
}

template <typename T>
inline std::vector<T>* RingBuffer<T>::get_write_buffer()
{
    /* Returns pointer to current write buffer */
    return &(this->ring[this->writeIndex]);
}

template <typename T>
std::vector<T>* RingBuffer<T>::get_processing_buffer()
{
    /* Returns pointer to current processing buffer */
    return &(this->ring[this->processingIndex]);
}

template <typename T>
inline uint8_t* RingBuffer<T>::get_read_byte()
{
    /* Returns pointer to first byte of current read buffer */
    return reinterpret_cast<uint8_t*>(
            &(this->ring[this->readIndex][0])
        );
}

template <typename T>
inline uint8_t* RingBuffer<T>::get_write_byte()
{
    /* Returns pointer to first byte of current write buffer */
    return reinterpret_cast<uint8_t*>(
            &(this->ring[this->writeIndex][0])
        );
}

template <typename T>
uint8_t* RingBuffer<T>::get_processing_byte()
{
    /* Returns pointer to first byte of current processing buffer */
    return reinterpret_cast<uint8_t*>(
            &(this->ring[this->processingIndex][0])
        );
}

template <typename T>
inline const std::vector<T> RingBuffer<T>::_read() const
{
    /* Returns current read buffer */
    return this->ring[this->readIndex];
}

template <typename T>
const std::vector<T> RingBuffer<T>::read(bool force)
{
    /* Returns current read buffer and rotates */
    #ifdef _DEBUG
    if (!_size_is_set()) throw BUFFER_NOT_INITIALIZED;
    #endif

    std::vector<T> output(_read());
    rotate_read_buffer(force);
    return output;
}

template <typename T>
void RingBuffer<T>::read_bytes(uint8_t* data, size_t numBytes, bool force)
{
    /* Copies data from read buffer to data pointer */
    #ifdef _DEBUG
    if (!_size_is_set()) throw BUFFER_NOT_INITIALIZED;
    #endif
    
    std::vector<T> output(_read());
    uint8_t* position = reinterpret_cast<uint8_t*>(&(output[0]));
    
    for (size_t i(0); i < numBytes; ++i)
    {
        data[i] = position[i];
    }
    
    rotate_read_buffer(force);
}

template <typename T>
void RingBuffer<T>::read_samples(T* data, size_t length, bool force)
{
    /* Read specified number of samples */
    #ifdef _DEBUG
    if (!_size_is_set()) throw BUFFER_NOT_INITIALIZED;
    #endif

    std::vector<T> output(_read());
    
    for (size_t i(0); i < length; ++i)
    {
        data[i] = output[i];
    }
    
    rotate_read_buffer(force);
}

template <typename T>
const std::vector<T> RingBuffer<T>::read_initial(bool force)
{
    rotate_read_index();
    return read(force);
}

template <typename T>
void RingBuffer<T>::read_bytes_initial(uint8_t* data, size_t numBytes, bool force)
{
    rotate_read_index();
    return read_bytes(data, numBytes, force);
}

template <typename T>
void RingBuffer<T>::read_samples_initial(T* data, size_t length, bool force)
{
    rotate_read_index();
    return read_samples(data, length, force);
}

template <typename T>
int RingBuffer<T>::write(T data, bool force)
{
    /* Write a single sample */
    #ifdef _DEBUG
    if (!_size_is_set()) throw BUFFER_NOT_INITIALIZED;
    #endif

    if (!is_writable() && !force) return 0;
    this->ring[this->writeIndex][this->_samplesWritten] = data;
    ++this->_samplesWritten;
    if (--this->_samplesRemaining <= 0)
    {
        rotate_write_buffer();
    }
    // ++this->_buffered;
    return 1;
}

template <typename T>
int RingBuffer<T>::write(std::vector<T> data, bool force)
{
    /* Writes along ring and returns total number of samples written.
    If forced, unread data will be overwritten. */
    int written(0), remaining(data.size());
    int8_t index(this->ringLength);
    while ((remaining > 0) && (is_writable() || force) && (index-- > 0))
    {
        if (remaining > this->_samplesRemaining)
        {
            std::copy(
                    data.begin() + written,
                    data.begin() + written + this->_samplesRemaining,
                    this->ring[this->writeIndex].begin() + this->_samplesWritten
                );
            written += this->_samplesRemaining;
            remaining -= this->_samplesRemaining;
            rotate_write_buffer();
        }
        else
        {
            std::copy(
                    data.begin() + written,
                    data.end(),
                    this->ring[this->writeIndex].begin() + this->_samplesWritten
                );
            this->_samplesWritten += remaining;
            this->_samplesRemaining -= remaining;
            written += remaining;
            remaining = 0;
            if (this->_samplesRemaining <= 0)
            {
                rotate_write_buffer();
            }
        }
    }
    return written;
}

template <typename T>
size_t RingBuffer<T>::write_bytes(uint8_t* data, size_t numBytes, bool force)
{
    /* Casts to std::vector and returns number of samples written.
    If forced, unread data will be overwritten. */
    std::vector<T> converted(
            reinterpret_cast<T*>(data),
            reinterpret_cast<T*>(data + numBytes)
        );
    return write(converted, force);
}

template <typename T>
size_t RingBuffer<T>::write_samples(T* data, size_t length, bool force)
{
    /* Write specified number of samples */
    std::vector<T> converted(data, data + length);
    return write(converted, force);
}

template <typename T>
int RingBuffer<T>::write_initial(T data, bool force)
{
    rotate_processing_index();
    return write(data, force);
}

template <typename T>
int RingBuffer<T>::write_initial(std::vector<T> data, bool force)
{
    rotate_processing_index();
    return write(data, force);
}

template <typename T>
size_t RingBuffer<T>::write_bytes_initial(uint8_t* data, size_t numBytes, bool force)
{
    rotate_processing_index();
    return write_bytes(data, numBytes, force);
}

template <typename T>
size_t RingBuffer<T>::write_samples_initial(T* data, size_t length, bool force)
{
    rotate_processing_index();
    return write_samples(data, length, force);
}

template <typename T>
inline void RingBuffer<T>::_set_buffer_processed(uint8_t ringIndex, bool state)
{
    /* Set whether a specified buffer has been processed or not */
    #if _DEBUG
    if (ringIndex >= this->ringLength) throw std::out_of_range("Buffer not found");
    #endif

    this->bufferProcessedState[ringIndex] = state;
}

template <typename T>
void RingBuffer<T>::set_buffer_processed(std::vector<T>* bufferPtr, bool state)
{
    /* Set whether the specified buffer has been processed */
    _set_buffer_processed(get_ring_index(bufferPtr), state);
}

template <typename T>
void RingBuffer<T>::set_buffer_processed(uint8_t* bufferPtr, bool state)
{
    /* Set whether the specified buffer has been processed */
    _set_buffer_processed(get_ring_index(bufferPtr), state);
}

template <typename T>
bool RingBuffer<T>::_is_buffer_processed(uint8_t ringIndex)
{
    /* Returns whether the specified buffer has been processed */
    return this->bufferProcessedState[ringIndex];
}

template <typename T>
bool RingBuffer<T>::is_buffer_processed(std::vector<T>* bufferPtr)
{
    /* Returns whether the specified buffer has been processed */
    return _is_buffer_processed(get_ring_index(bufferPtr));
}

template <typename T>
bool RingBuffer<T>::is_buffer_processed(uint8_t* bufferPtr)
{
    /* Returns whether the specified buffer has been processed */
    return _is_buffer_processed(get_ring_index(bufferPtr));
}

template class Buffer::RingBuffer<int8_t>;
template class Buffer::RingBuffer<uint8_t>;
template class Buffer::RingBuffer<int16_t>;
template class Buffer::RingBuffer<uint16_t>;
template class Buffer::RingBuffer<int32_t>;
template class Buffer::RingBuffer<uint32_t>;
template class Buffer::RingBuffer<int64_t>;
template class Buffer::RingBuffer<uint64_t>;
template class Buffer::RingBuffer<float>;
template class Buffer::RingBuffer<double>;
template class Buffer::RingBuffer<long double>;

template class Buffer::RingBuffer<char>;
template class Buffer::RingBuffer<char16_t>;
template class Buffer::RingBuffer<char32_t>;
