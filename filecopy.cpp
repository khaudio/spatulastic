#include "filecopy.h"


FileMover::FileMover() :
_numBytesReadToBuffer(0),
_numBytesWrittenFromBuffer(0),
_buff(CHUNKSIZE, 4),
_firstRead(false),
_firstWritten(false),
_lastBuff(false),
_lastWritten(false),
started(false)
{
}

FileMover::~FileMover()
{
    close();
}

bool FileMover::ready()
{
    return (this->_inStream.is_open() && this->_outStream.is_open());
}

void FileMover::open_source(const char* filepath)
{
    this->_inStream.open(filepath, std::ios::binary);
}

void FileMover::open_dest(const char* filepath)
{
    this->_outStream.open(filepath, std::ios::binary);
}

void FileMover::close()
{
    this->_inStream.close();
    this->_outStream.close();
}

size_t FileMover::bytes_remaining()
{
    return this->_numBytesReadToBuffer - this->_numBytesWrittenFromBuffer;
}

bool FileMover::complete()
{
    return (
            this->started
            && !ready()
//             && this->_lastWritten
            && !bytes_remaining()
            && !this->_buff.buffered()
        );
}

void FileMover::_print_status()
{
    std::cout << "bytes read: " << this->_numBytesReadToBuffer << std::endl;
    std::cout << "bytes written: " << this->_numBytesWrittenFromBuffer << std::endl;
    std::cout << "bytes remaining: " << bytes_remaining() << std::endl;
    std::cout << "samples buffered: " << this->_buff.buffered() << std::endl;
    std::cout << "samples available: " << this->_buff.available() << std::endl;
    
    std::cout << " buffers: " << std::endl;
    int i(1);
    for (auto& buff: this->_buff.ring)
    {
        std::cout << "buff # " << i++ << std::endl;
        std::cout << "'''";
        for (auto& c: buff)
        {
            std::cout << c;
        }
        std::cout << "'''";
        if ((uint8_t*)&buff[0] == this->_buff.get_read_byte())
        {
            std::cout << " ----> OUT FROM BUFFER";
        }
        if ((uint8_t*)&buff[0] == this->_buff.get_write_byte())
        {
            std::cout << " <---- IN TO BUFFER";
        }
        std::cout << "\n\n";
    }
}

size_t FileMover::_read_to_buffer()
{
    static int i(0);

    size_t numBytesRead;
    
    if (!this->_inStream.is_open()) return 0;
    
    this->_inStream.read(this->_tempReadBuff, this->_buff.bufferLength);
    
    if (!i++) std::cout << "first read:\t" << this->_tempReadBuff << std::endl;
    
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
        std::cout << "pumping lastBuff: " << this->_tempReadBuff << std::endl;
        this->_lastBuff = true;
    }
    
    this->_numBytesReadToBuffer += numBytesRead;
    
    return numBytesRead;
}

size_t FileMover::read_loop()
{
    return 0;
}

size_t FileMover::_write_from_buffer()
{
    size_t numBytesWritten(0);
    size_t beforePosition, afterPosition(0);
    static size_t lastPosition(0);
    
    static int i(0);
    
    beforePosition = this->_outStream.tellp();
    
    if (!this->_inStream.is_open() && (this->_buff.buffered() >= this->_buff.bufferLength))
    {
        this->_buff.read(this->_tempWriteBuff, this->_buff.bufferLength);
        
        std::cout << i++ << " flushing buffers\n";
        std::cout << this->_tempWriteBuff << std::endl;
        
        this->_outStream.write(this->_tempWriteBuff, this->_buff.bufferLength);
        afterPosition = this->_outStream.tellp();
    }
    else if (this->_lastBuff && !this->_lastWritten && !this->_buff.buffered())
    {
        this->_outStream.write(this->_tempReadBuff, bytes_remaining());
        afterPosition = this->_outStream.tellp();
        this->_lastWritten = true;
        
        std::cout << "dumping temp read buff:\n";
        std::cout << this->_tempReadBuff;
        std::cout << std::endl;
        
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
        
        if (!i) std::cout << "first write:\t" << this->_tempWriteBuff << std::endl;
        std::cout << i++ << std::endl;
        std::cout << this->_tempWriteBuff;
        std::cout << std::endl;
        
        this->_outStream.write(this->_tempWriteBuff, this->_buff.bufferLength);
        afterPosition = this->_outStream.tellp();
    }
    
    if (this->_outStream.is_open() && (afterPosition > beforePosition))
    {
        std::cout << "after " << afterPosition << " before " << beforePosition << "\n";
        std::cout << "last " << lastPosition << std::endl;
        
        lastPosition = afterPosition;
        numBytesWritten += (afterPosition - beforePosition);
        this->_numBytesWrittenFromBuffer += numBytesWritten;
        
        std::cout << "numBytesWritten " << numBytesWritten << "\n"; 
    }
    else if ((afterPosition < beforePosition) || this->_lastWritten)
    {
        
        /* close stream here? */
        
        this->_outStream.close();
        
        std::cout << "mismatch" << std::endl;
        std::cout << "after " << afterPosition << " before " << beforePosition << "\n";
        std::cout << "last " << lastPosition << std::endl;
    }
    
    return numBytesWritten;
}

size_t FileMover::write_loop()
{
    return 0;
}

size_t FileMover::execute()
{
    this->started = true;
    
    while (this->_buff.available() && this->_inStream.is_open())
    {
        _print_status();
        _read_to_buffer();
    }
    
    while (!complete())
    {
        _write_from_buffer();
        _read_to_buffer();
        
        _print_status();
    }
    
    close();
    
    /*#ifdef DEBUG
    if (this->_numBytesReadToBuffer != this->_numBytesWrittenFromBuffer)
    {
        throw READ_WRITE_MISMATCH;
    }
    #endif*/
    
    std::cout << "read " << this->_numBytesReadToBuffer << std::endl;
    std::cout << "write " << this->_numBytesWrittenFromBuffer << std::endl;
    
    return this->_numBytesWrittenFromBuffer;
}