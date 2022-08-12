#include "filecopy.h"


FileCopy::FileCopy() :
_firstWritten(false),
_lastBuff(false),
_lastWritten(false),
_numBytesReadToBuffer(0),
_numBytesWrittenFromBuffer(0),
_buff(CHUNKSIZE, 4),
started(false)
{
}

FileCopy::~FileCopy()
{
    close();
}

bool FileCopy::ready()
{
    return (this->_inStream.is_open() && this->_outStream.is_open());
}

void FileCopy::open_source(const char* filepath)
{
    this->_inStream.open(filepath, std::ios::binary);
}

void FileCopy::open_dest(const char* filepath)
{
    this->_outStream.open(filepath, std::ios::binary);
}

void FileCopy::close()
{
    this->_inStream.close();
    this->_outStream.close();
}

size_t FileCopy::bytes_remaining()
{
    return this->_numBytesReadToBuffer - this->_numBytesWrittenFromBuffer;
}

bool FileCopy::complete()
{
    return (
            this->started
            && !ready()
            && !bytes_remaining()
            && !this->_buff.buffered()
        );
}

size_t FileCopy::_read_to_buffer()
{
    size_t numBytesRead;
    
    if (!this->_inStream.is_open()) return 0;
    
    this->_inStream.read(this->_tempReadBuff, this->_buff.bufferLength);
    
    numBytesRead = this->_inStream.gcount();
    
    if (!numBytesRead)
    {
        this->_inStream.close();
    }
    else if (numBytesRead == this->_buff.bufferLength)
    {
        this->_buff.write(this->_tempReadBuff, numBytesRead);
    }
    else
    {
        this->_lastBuff = true;
    }

    this->_numBytesReadToBuffer += numBytesRead;
    
    return numBytesRead;
}

size_t FileCopy::read_loop()
{
    while (!complete())
    {
        _read_to_buffer();
        std::this_thread::yield();
    }
    return this->_numBytesReadToBuffer;
}

size_t FileCopy::_write_from_buffer()
{
    size_t numBytesWritten(0);
    size_t beforePosition, afterPosition(0);
    
    beforePosition = this->_outStream.tellp();

    if (
            !this->_inStream.is_open()
            && (this->_buff.buffered() >= this->_buff.bufferLength)
        )
    {
        this->_buff.read(this->_tempWriteBuff, this->_buff.bufferLength);
        
        this->_outStream.write(this->_tempWriteBuff, this->_buff.bufferLength);
        afterPosition = this->_outStream.tellp();
    }
    else if (this->_lastBuff && !this->_lastWritten && !this->_buff.buffered())
    {
        this->_outStream.write(this->_tempReadBuff, bytes_remaining());
        afterPosition = this->_outStream.tellp();
        this->_lastWritten = true;
    }
    else if (this->_buff.buffered())
    {
        if (!this->_firstWritten)
        {
            this->_buff.rotate_read_buffer();
            this->_buff._buffered += this->_buff.bufferLength;
            this->_firstWritten = true;
        }
        this->_buff.read(this->_tempWriteBuff, this->_buff.bufferLength);
        this->_outStream.write(this->_tempWriteBuff, this->_buff.bufferLength);
        afterPosition = this->_outStream.tellp();
    }
    
    if (this->_outStream.is_open() && (afterPosition > beforePosition))
    {
        numBytesWritten += (afterPosition - beforePosition);
        this->_numBytesWrittenFromBuffer += numBytesWritten;
    }
    else if ((afterPosition < beforePosition) || this->_lastWritten)
    {
        this->_outStream.close();
    }
    
    return numBytesWritten;
}

size_t FileCopy::write_loop()
{
    while (!complete())
    {
        _write_from_buffer();
        std::this_thread::yield();
    }
    return this->_numBytesWrittenFromBuffer;
}

size_t FileCopy::execute()
{
    this->started = true;
    
    while (this->_buff.available() && this->_inStream.is_open())
    {
        _read_to_buffer();
    }
    while (!complete())
    {
        _write_from_buffer();
        _read_to_buffer();
    }
    
    close();
    
    #ifdef _DEBUG
    if (this->_numBytesReadToBuffer != this->_numBytesWrittenFromBuffer)
    {
        throw READ_WRITE_MISMATCH;
    }
    #endif

    return this->_numBytesWrittenFromBuffer;
}
