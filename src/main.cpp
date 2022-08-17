#include "filecopy.h"
#include "progressbar.h"
#include "ringbuffer.h"
#include "hashlibpp.h"


enum spatulastic_err
{
    CHECKSUM_MISMATCH = 1001,
};


void hash_data(FileCopy* fc, MD5* hasher, HL_MD5_CTX* hasherCtx)
{
    /* Hashes binary data */
    size_t numBytes = (
            fc->_buff.buffered() >= fc->_buff.bufferLength
            ? fc->_buff.bytesPerBuffer
            : fc->_buff.buffered() * fc->_buff.bytesPerSample
        );
    hasher->MD5Update(hasherCtx, fc->_buff.get_processing_byte(), numBytes);
    fc->_buff.rotate_partial_processing(numBytes);
}

std::string get_checksum_from_hasher(MD5* hasher, HL_MD5_CTX* hasherCtx)
{
    /* Generate a checksum from the hashed data */
    std::ostringstream stream;
    uint8_t buff[16] = "";
    hasher->MD5Final(
            (uint8_t*)(buff),
            hasherCtx
        );
    for (int i(0); i < 16; ++i)
    {
        stream.width(2);
        stream.fill('0');
        stream << std::hex << static_cast<unsigned int>(buff[i]);
    }
    return stream.str();
}

void execute_transfer(FileCopy* fc, bool sourceHashInline)
{
    /* Execute transfer with a progress bar
    and generate checksums */

    BasicProgressBar<double> bar;
    MD5 sourceHasher;
    HL_MD5_CTX sourceHasherCtx;
    
    md5wrapper sourceHashWrapper;
    md5wrapper destHashWrapper;

    std::string sourceHash, destHash;
    char status[40];

    /* Get the file size for the progress bar */
    bar.set_maximum(fc->get_source_size());

    /* Initialize the source hasher */
    sourceHasher.MD5Init(&sourceHasherCtx);

    /* Pre-buffer one chunk from the source file */
    fc->read_to_buffer();

    std::cout << "Starting transfer loop..." << std::endl;

    while (!fc->complete())
    {
        size_t bytesMoved(0);

        /* Read data into buffer */
        fc->read_to_buffer();
        
        if (sourceHashInline)
        {
            /* Hash the source file data as we cycle
            it through the buffer */
            hash_data(fc, &sourceHasher, &sourceHasherCtx);
        }

        /* Write the data out to the destination file */
        if (sourceHashInline)
        {
            bytesMoved = fc->write_processed_from_buffer();
        }
        else
        {
            bytesMoved = fc->write_from_buffer();
        }

        /* Get a progress bar and print it */
        bar.increment(bytesMoved);
        bar.get_bar(status, sizeof(status));

        for (size_t i(0); i < sizeof(status); ++i)
        {
            std::cout << status[i];
        }
        std::cout << "\r";
    }
    std::cout << std::endl;

    std::cout << "Closing file..." << std::endl;

    /* Close the files */
    fc->close();

    fc->_destSizeInBytes = fc->get_file_size(fc->dest);
    if (fc->_sourceSizeInBytes != fc->_destSizeInBytes)
    {
        throw FILESIZE_MISMATCH;
    }
    #ifdef _DEBUG
    if (fc->_numBytesReadToBuffer != fc->_numBytesWrittenFromBuffer)
    {
        throw READ_WRITE_MISMATCH;
    }
    #endif

    std::cout << "Generating source checksum... ";

    if (sourceHashInline)
    {
        /* Generate the source checksum */
        sourceHash = get_checksum_from_hasher(&sourceHasher, &sourceHasherCtx);
    }
    else
    {
        /* Generate the source file checksum all at once
        from the file instead of the buffer */
        sourceHash = sourceHashWrapper.getHashFromFile(fc->source.c_str());
    }
    
    std::cout << "Done." << std::endl;
    std::cout << "Generating destination checksum... ";

    /* Get destination file checksum now that the transfer is complete */
    destHash = destHashWrapper.getHashFromFile(fc->dest.c_str());

    std::cout << "Done." << std::endl;
    std::cout << std::setw(32) << "Source checksum:\t" << sourceHash << std::endl;
    std::cout << std::setw(32) << "Destination checksum:\t" << destHash << std::endl;

    /* Compare source and destination file checksums */
    if (sourceHash != destHash)
    {
        throw CHECKSUM_MISMATCH;
    }
}

int main(int argc, char** argv)
{
    using namespace std::chrono_literals;

    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::high_resolution_clock::duration duration;
 
    start = std::chrono::high_resolution_clock::now();

    /* It's, umm... it's main() */
    FileCopy fc;

    /* Whether to hash the source file
    while it's being moved in the ring buffer */
    bool inlineHash = true;
    
    std::cout << "Starting spatulastic..." << std::endl;
    std::cout << "Opening files... ";

    fc.open_source("../demo_random_data");
    fc.open_dest();

    std::cout << "Done" << std::endl;
    std::cout << "Starting transfer..." << std::endl;

    execute_transfer(&fc, inlineHash);

    std::cout << "Transfer complete" << std::endl;
    std::cout << "Spatulastic out!" << std::endl;

    end = std::chrono::high_resolution_clock::now();

    duration = end - start;

    std::cout << "Transfer took ";
    std::cout << static_cast<double>(duration.count()) / 1000000;
    std::cout << " ms" << std::endl;

    std::ofstream log;
    log.open("log.txt", std::ios::app);
    log << "Source hashed " << (inlineHash ? "inline" : "after") << "\t";
    log << static_cast<double>(duration.count()) / 1000000 << " ms" << std::endl;
    log.close();

    return 0;
}
