#include "filecopy.h"


FileCopy::FileCopy() :
_firstWritten(false),
_sourceSizeInBytes(0),
_destSizeInBytes(0),
_numBytesReadToBuffer(0),
_numBytesWrittenFromBuffer(0),
_buff(RING_BUFFER_CHUNKSIZE, 4),
started(false)
{
}

FileCopy::~FileCopy()
{
    close();
}

bool FileCopy::ready()
{
    /* Indicates if both files are open and ready to transfer */
    return (this->_inStream.is_open() && this->_outStream.is_open());
}

size_t FileCopy::get_file_size(const char* filepath)
{
    /* Gets file size of specified filepath */
    return std::filesystem::file_size(std::filesystem::path(filepath));
}

size_t FileCopy::get_file_size(std::filesystem::path filepath)
{
    /* Gets file size of specified filepath */
    return std::filesystem::file_size(filepath);
}

void FileCopy::set_dest_dir(const char* directory)
{
    this->dest = directory;
}

void FileCopy::open_source(const char* filepath)
{
    /* Opens source file and gets file size */
    this->source = filepath;
    
    get_source_size();
    this->_inStream.open(this->source, std::ios::binary);
}

void FileCopy::open_dest(const char* filepath)
{
    /* Opens new destination file */
    this->dest = filepath;
    this->_outStream.open(this->dest, std::ios::binary);
}

void FileCopy::open_dest()
{
    /* Opens new destination file
    with the same base filename as source file */
    #if _DEBUG
    if (this->source.empty())
    {
        throw SOURCE_NOT_SET;
    }
    #endif

    this->dest.replace_filename(this->source.filename());
    this->_outStream.open(this->dest, std::ios::binary);
}

size_t FileCopy::get_source_size()
{
    /* Get source file size */
    if (!this->_sourceSizeInBytes)
    {
        this->_sourceSizeInBytes = get_file_size(this->source);
    }
    return this->_sourceSizeInBytes;
}

size_t FileCopy::get_dest_size()
{
    /* Get destination file size only if transfer has completed */
    if (!complete()) return 0;
    if (!this->_destSizeInBytes)
    {
        this->_destSizeInBytes = get_file_size(this->dest);
    }
    return this->_destSizeInBytes;
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

size_t FileCopy::read_to_buffer()
{
    /* Copies data from the source to the ring buffer */
    size_t numBytesRead;
    
    if (!this->started)
    {
        this->started = true;
        this->_buff.rotate_processing_index();
    }
    
    if (!this->_inStream.is_open() || !this->_buff.buffers_available()) return 0;

    char* bufferWriteByte = reinterpret_cast<char*>(this->_buff.get_write_byte());
    this->_inStream.read(bufferWriteByte, this->_buff.bufferLength);

    numBytesRead = this->_inStream.gcount();
    
    if (!numBytesRead)
    {
        this->_inStream.close();
    }
    else
    {
        this->_buff.rotate_partial_write(numBytesRead);
    }

    this->_numBytesReadToBuffer += numBytesRead;

    return numBytesRead;
}

size_t FileCopy::write_from_buffer()
{
    /* Writes from the buffer out to the destination */
    size_t beforePosition, afterPosition(0), numBytesWritten(0);
    
    if (!this->started) this->started = true;
    if (!this->_outStream.is_open()) return 0;

    beforePosition = this->_outStream.tellp();

    if (this->_buff.buffered() > 0)
    {
        if (!this->_firstWritten)
        {
            this->_buff.rotate_read_index();
            this->_firstWritten = true;
        }

        int numBytesBuffered = (
                (this->_buff.buffered() >= this->_buff.bufferLength)
                ? this->_buff.bufferLength
                : this->_buff.buffered()
            );

        char* bufferReadByte = reinterpret_cast<char*>(this->_buff.get_read_byte());
        this->_outStream.write(bufferReadByte, numBytesBuffered);
        afterPosition = this->_outStream.tellp();
        numBytesWritten += (afterPosition - beforePosition);
        this->_buff.rotate_partial_read(numBytesWritten);
        this->_numBytesWrittenFromBuffer += numBytesWritten;
    }

    if (this->_outStream.is_open() && (afterPosition < beforePosition))
    {
        this->_outStream.close();
    }

    return numBytesWritten;
}

size_t FileCopy::write_processed_from_buffer()
{
    /* Writes from the buffer out to the destination */
    size_t beforePosition, afterPosition(0), numBytesWritten(0);
    
    if (!this->started) this->started = true;
    if (!this->_outStream.is_open()) return 0;

    beforePosition = this->_outStream.tellp();

    if (this->_buff.processed() > 0)
    {
        if (!this->_firstWritten)
        {
            this->_buff.rotate_read_index();
            this->_firstWritten = true;
        }

        int numBytesBuffered = (
                (this->_buff.processed() >= this->_buff.bufferLength)
                ? this->_buff.bufferLength
                : this->_buff.processed()
            );

        char* bufferReadByte = reinterpret_cast<char*>(this->_buff.get_read_byte());
        this->_outStream.write(bufferReadByte, numBytesBuffered);
        afterPosition = this->_outStream.tellp();
        numBytesWritten += (afterPosition - beforePosition);
        this->_buff.rotate_partial_read(numBytesWritten);
        this->_numBytesWrittenFromBuffer += numBytesWritten;
    }

    if (this->_outStream.is_open() && (afterPosition < beforePosition))
    {
        this->_outStream.close();
    }

    return numBytesWritten;
}

size_t FileCopy::execute()
{
    #if _DEBUG
    if (this->source.empty())
    {
        throw SOURCE_NOT_SET;
    }
    if (this->dest.empty())
    {
        throw DEST_NOT_SET;
    }
    #endif

    /* Executes copy and blocks until transfer is complete */
    this->started = true;

    while (this->_buff.available() && this->_inStream.is_open())
    {
        read_to_buffer();
    }
    while (!complete())
    {
        write_from_buffer();
        read_to_buffer();
    }

    close();
    
    this->_destSizeInBytes = get_file_size(this->dest);

    if (this->_sourceSizeInBytes != this->_destSizeInBytes)
    {
        throw FILESIZE_MISMATCH;
    }

    #ifdef _DEBUG
    if (this->_numBytesReadToBuffer != this->_numBytesWrittenFromBuffer)
    {
        throw READ_WRITE_MISMATCH;
    }
    #endif

    return this->_numBytesWrittenFromBuffer;
}
