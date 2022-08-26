#include "filecopy.h"


FileCopy::FileCopy() :
_firstWritten(false),
_overwrite(false),
_sourceSizeInBytes(0),
_destSizeInBytes(0),
_numBytesReadToBuffer(0),
_numBytesWrittenFromBuffer(0),
_buff(RING_BUFFER_CHUNKSIZE, 4),
started(false)
{
}

FileCopy::FileCopy(const FileCopy& obj) :
_firstWritten(obj._firstWritten),
_overwrite(obj._overwrite),
_sourceSizeInBytes(obj._sourceSizeInBytes),
_destSizeInBytes(obj._destSizeInBytes),
_numBytesReadToBuffer(obj._numBytesReadToBuffer),
_numBytesWrittenFromBuffer(obj._numBytesWrittenFromBuffer),
_buff(obj._buff.bufferLength, obj._buff.ringLength),
started(obj.started)
{
}

FileCopy::~FileCopy()
{
    close();
}

inline void FileCopy::_check_paths_not_empty()
{
    if (this->source.empty())
    {
        throw SOURCE_NOT_SET;
    }
    if (this->dest.empty())
    {
        throw DEST_NOT_SET;
    }
}

inline void FileCopy::_check_buffer_match()
{
    if (this->_numBytesReadToBuffer != this->_numBytesWrittenFromBuffer)
    {
        throw READ_WRITE_MISMATCH;
    }
}

inline void FileCopy::_check_file_size_match()
{
    if (this->_sourceSizeInBytes != this->_destSizeInBytes)
    {
        throw FILESIZE_MISMATCH;
    }
}

void FileCopy::open_source(std::filesystem::path filepath)
{
    /* Opens source file and gets file size */
    this->source = filepath;
    get_source_size();
    this->_inStream.open(this->source, std::ios::binary);
}

void FileCopy::open_source(const char* filepath)
{
    /* Opens source file and gets file size */
    this->source = filepath;
    get_source_size();
    this->_inStream.open(this->source, std::ios::binary);
}

inline void FileCopy::open_dest(std::filesystem::path filepath)
{
    /* Opens new destination file */
    this->dest = filepath;
    if (!this->_overwrite && std::filesystem::exists(this->dest))
    {
        get_dest_size();
        return;
    }
    this->_outStream.open(this->dest, std::ios::binary);
}

void FileCopy::open_dest(const char* filepath)
{
    /* Opens new destination file */
    open_dest(std::filesystem::path(filepath));
}

std::filesystem::path FileCopy::_rename_dest()
{
    #if _DEBUG
    if (this->source.empty())
    {
        throw SOURCE_NOT_SET;
    }
    #endif
    this->dest.replace_filename(this->source.filename());
}

void FileCopy::open_dest()
{
    open_dest(std::filesystem::path(_rename_dest()));
}

size_t FileCopy::get_source_size()
{
    /* Get source file size */
    if (!this->_sourceSizeInBytes)
    {
        this->_sourceSizeInBytes = std::filesystem::file_size(this->source);
    }
    return this->_sourceSizeInBytes;
}

size_t FileCopy::get_dest_size()
{
    /* Get destination file size */
    #if _DEBUG
    std::cout << "Checking dest file size" << std::endl;
    if (started && !complete())
    {
        std::cerr << "Cannot get dest size; ";
        std::cerr << "incomplete copy in progress" << std::endl;
        return 0;
    }
    #endif
    if (!this->_destSizeInBytes)
    {
        this->_destSizeInBytes = std::filesystem::file_size(this->dest);
    }
    return this->_destSizeInBytes;
}

size_t FileCopy::bytes_remaining()
{
    return this->_numBytesReadToBuffer - this->_numBytesWrittenFromBuffer;
}

bool FileCopy::ready()
{
    /* Indicates if both files are open and ready to transfer */
    return (this->_inStream.is_open() && this->_outStream.is_open());
}

void FileCopy::close()
{
    #if _DEBUG
    std::cout << "Closing files..." << std::endl;
    #endif
    this->_inStream.close();
    this->_outStream.close();
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

void FileCopy::reset()
{
    close();
    this->source = std::filesystem::path();
    this->dest = std::filesystem::path();
    this->started = false;
    this->_firstWritten = false;
    this->_sourceSizeInBytes = 0;
    this->_destSizeInBytes = 0;
    this->_numBytesReadToBuffer = 0;
    this->_numBytesWrittenFromBuffer = 0;
    this->_buff.reset();
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
    /* Writes data from the buffer out to the destination
    only if it has been processed while in the buffer */
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

void FileCopy::pre_buffer_source()
{
    while (this->_buff.available() && this->_inStream.is_open())
    {
        read_to_buffer();
    }
}

size_t FileCopy::execute()
{
    #if _DEBUG
    std::cout << "Executing transfer:" << std::endl;
    std::cout << "Copying\n\t" << this->source.string();
    std::cout << "\n\tto\n\t" << this->dest.string();
    std::cout << std::endl;
    _check_paths_not_empty();
    #endif

    if (this->_destSizeInBytes && !this->_overwrite)
    {
        /* If destination file exists, skip it */
        #if _DEBUG
        std::cout << "Destination file exists; skipping" << std::endl;
        #endif
        goto copyComplete;
    }
    #if _DEBUG
    else if (this->_destSizeInBytes && this->_overwrite)
    {
        /* If destination file exists, overwrite it */
        std::cout << "Destination file exists; overwriting: ";
        std::cout << this->dest.string() << std::endl;
    }
    else if (!this->_destSizeInBytes)
    {
        /* If destination file does not exist, create it */
        std::cout << "Creating destination file: ";
        std::cout << this->dest.string() << std::endl;
    }
    #endif

    /* Executes copy and blocks until transfer is complete */
    this->started = true;

    pre_buffer_source();
    while (!complete())
    {
        write_from_buffer();
        read_to_buffer();
    }

    #if _DEBUG
    _check_buffer_match();
    #endif

    /* Jump to here if existing file was skipped */
    copyComplete:

    close();
    get_dest_size();

    #ifdef _DEBUG
    _check_file_size_match();
    std::cout << "Execute completed" << std::endl;
    #endif

    /* Return the actual file size */
    return this->_destSizeInBytes;
}
