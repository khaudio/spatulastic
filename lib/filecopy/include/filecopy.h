#include <fstream>

#include "ringbuffer.h"

#ifndef RING_BUFFER_CHUNKSIZE
    #define RING_BUFFER_CHUNKSIZE           ((1024)*(1024))
#endif

enum file_mover_err
{
    READ_WRITE_MISMATCH = 101,
    WRITEFILE_OVERFLOW = 102,
    BUFFERED_DATA_MISMATCH = 103,
};


class FileCopy
{
protected:

    bool _firstWritten;

    size_t _numBytesReadToBuffer, _numBytesWrittenFromBuffer;

    char _inPath[4096], _outPath[4096];

    std::ifstream _inStream;
    std::ofstream _outStream;

    Buffer::RingBuffer<char> _buff;

public:

    bool started;

    FileCopy();
    ~FileCopy();

    void open_source(const char* filepath);
    void open_dest(const char* filepath);
    void close();
    
    bool ready();
    bool complete();

    size_t bytes_remaining();

    size_t read_to_buffer();
    size_t write_from_buffer();
    size_t execute();
};
