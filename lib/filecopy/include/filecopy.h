#include <fstream>
#include <iostream>
#include <thread>
#include <iostream>
#include "ringbuffer.h"

#ifndef DEBUG
#define DEBUG
#endif



#define CHUNKSIZE    3


enum file_mover_err
{
    READ_WRITE_MISMATCH = 101,
    WRITEFILE_OVERFLOW = 102,
    BUFFERED_DATA_MISMATCH = 103,
};


class FileMover
{
protected:
    size_t _numBytesReadToBuffer, _numBytesWrittenFromBuffer;
    std::ifstream _inStream;
    std::ofstream _outStream;
    char _inPath[4096], _outPath[4096];
    Buffer::RingBuffer<char> _buff;
    char _tempReadBuff[CHUNKSIZE], _tempWriteBuff[CHUNKSIZE];
    bool _firstRead, _firstWritten, _lastBuff, _lastWritten;
    
    void _print_status();
    
public:
    bool started;

    FileMover();
    ~FileMover();

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