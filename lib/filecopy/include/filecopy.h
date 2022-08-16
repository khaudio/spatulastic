#ifndef FILECOPY_H
#define FILECOPY_H

#include <fstream>
#include <filesystem>

#include "ringbuffer.h"

#ifndef RING_BUFFER_CHUNKSIZE
    #define RING_BUFFER_CHUNKSIZE           ((1024)*(1024))
#endif

enum file_mover_err
{
    READ_WRITE_MISMATCH = 101,
    FILESIZE_MISMATCH = 102,
    WRITEFILE_OVERFLOW = 103,
    BUFFERED_DATA_MISMATCH = 104,
};


class FileCopy
{
protected:
public:
    bool _firstWritten;

    size_t
        _sourceSizeInBytes,
        _destSizeInBytes,
        _numBytesReadToBuffer,
        _numBytesWrittenFromBuffer;

    std::filesystem::path source, dest;

    std::ifstream _inStream;
    std::ofstream _outStream;

    Buffer::RingBuffer<char> _buff;

public:
    bool started;

    FileCopy();
    ~FileCopy();
    
    static size_t get_file_size(const char* filepath);
    static size_t get_file_size(std::filesystem::path filepath);
    
    void open_source(const char* filepath);
    void open_dest(const char* filepath);
    
    size_t get_source_size();
    size_t get_dest_size();
    
    void close();
    
    bool ready();
    bool complete();

    size_t bytes_remaining();

    size_t read_to_buffer();
    size_t write_from_buffer();
    size_t execute();
};

#endif
