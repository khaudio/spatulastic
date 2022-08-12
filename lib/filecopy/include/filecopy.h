#include <fstream>
#include <thread>

#include "ringbuffer.h"


#define CHUNKSIZE       (1024 * 1024)


enum file_mover_err
{
    READ_WRITE_MISMATCH = 101,
    WRITEFILE_OVERFLOW = 102,
    BUFFERED_DATA_MISMATCH = 103,
};


class FileCopy
{
protected:

    bool _firstWritten, _lastBuff, _lastWritten;
    size_t _numBytesReadToBuffer, _numBytesWrittenFromBuffer;
    char _inPath[4096], _outPath[4096];
    char _tempReadBuff[CHUNKSIZE], _tempWriteBuff[CHUNKSIZE];

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
    
    size_t _read_to_buffer();
    size_t read_loop();
    
    size_t _write_from_buffer();
    size_t write_loop();
    
    size_t execute();
};