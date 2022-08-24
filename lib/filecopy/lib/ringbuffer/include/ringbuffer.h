#ifndef RINGBUFFER_H
#define RINGBUFFER_H

#include <algorithm>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <iomanip>
#include <limits>
#include <vector>
#include <map>
#include <stdexcept>


namespace Buffer
{

enum ringbuffer_err
{
    RING_SIZE_TOO_SHORT = 140,
    BUFFER_NOT_INITIALIZED = 150,
};

template <typename T>
constexpr T get_zero()
{
    const uint64_t exponent = (sizeof(T) * 8) - 1;
    uint64_t value = 2;
    for (int i(0); i < exponent; ++i) value *= 2;
    return (
            std::numeric_limits<T>::is_integer
            && std::is_unsigned<T>()
        ) ? (value - 1) : 0;
}

class Ring
{
public:
    uint8_t ringLength;

    Ring();
    ~Ring();
};

template <typename T>
class RingBuffer : virtual public Ring
{
public:
    const static T _zero;

protected:
    int
        _totalWritableLength, _buffered,
        _samplesWritten, _samplesRemaining, _samplesProcessed;

    bool _size_is_set();

public:
    uint32_t
        bufferLength,
        totalRingSampleLength,
        bytesPerSample,
        bytesPerBuffer;
    uint8_t readIndex, writeIndex, processingIndex;
    std::vector<std::vector<T>> ring;
    std::map<uint8_t, bool> bufferProcessedState;

    RingBuffer();
    RingBuffer(int bufferSize, uint8_t ringSize);
    ~RingBuffer();

    virtual void set_size(int bufferSize, uint8_t ringSize);
    virtual size_t size();

    virtual void zero_fill();
    virtual void reset(bool zeroFill = false);

    virtual int buffered();
    virtual int available();
    virtual int processed();
    virtual int buffers_buffered();
    virtual int buffers_available();
    virtual int buffers_processed();
    virtual bool is_writable();

    virtual void rotate_read_index();
    virtual void rotate_write_index();
    virtual void rotate_processing_index();

    virtual void rotate_read_buffer(bool force = false);
    virtual void rotate_write_buffer(bool force = false);
    virtual void rotate_processing_buffer();

    virtual void rotate_partial_read(unsigned int length, bool force = false);
    virtual void rotate_partial_write(unsigned int length, bool force = false);
    virtual void rotate_partial_processing(unsigned int length);

    virtual uint8_t get_ring_index(std::vector<T>* bufferPtr);
    virtual uint8_t get_ring_index(uint8_t* bufferPtr);

    virtual std::vector<T>* get_read_buffer();
    virtual std::vector<T>* get_write_buffer();
    virtual std::vector<T>* get_processing_buffer();

    virtual uint8_t* get_read_byte();
    virtual uint8_t* get_write_byte();
    virtual uint8_t* get_processing_byte();

/*                               Read                               */

protected:
    virtual const std::vector<T> _read() const;

public:
    virtual const std::vector<T> read(bool force = false);
    virtual void read_bytes(uint8_t* data, size_t numBytes, bool force = false);
    virtual void read_samples(T* data, size_t length, bool force = false);

    /* Initial read once the buffer is initialized */
    virtual const std::vector<T> read_initial(bool force = false);
    virtual void read_bytes_initial(uint8_t* data, size_t numBytes, bool force = false);
    virtual void read_samples_initial(T* data, size_t length, bool force = false);

/*                               Write                              */

public:
    virtual int write(T data, bool force = false);
    virtual int write(std::vector<T> data, bool force = false);
    virtual size_t write_bytes(uint8_t* data, size_t numBytes, bool force = false);
    virtual size_t write_samples(T* data, size_t length, bool force = false);
    
    /* Initial write after data has been read to the buffer */
    virtual int write_initial(T data, bool force = false);
    virtual int write_initial(std::vector<T> data, bool force = false);
    virtual size_t write_bytes_initial(uint8_t* data, size_t numBytes, bool force = false);
    virtual size_t write_samples_initial(T* data, size_t length, bool force = false);

/*                             Transform                            */

protected:
    virtual void _set_buffer_processed(uint8_t ringIndex, bool state);
    virtual bool _is_buffer_processed(uint8_t ringIndex);

public:
    virtual void set_buffer_processed(std::vector<T>* bufferPtr, bool state);
    virtual void set_buffer_processed(uint8_t* bufferPtr, bool state);

    virtual bool is_buffer_processed(std::vector<T>* bufferPtr);
    virtual bool is_buffer_processed(uint8_t* bufferPtr);
};

};

#endif
