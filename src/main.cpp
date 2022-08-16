#include "filecopy.h"
#include "progressbar.h"
#include "ringbuffer.h"
#include "hashlibpp.h"


/* TODO
FileCopy::write_from_buffer()
    only pull processed() data instead of buffered()
*/


enum spatulastic_err
{
    CHECKSUM_MISMATCH = 1001,
};


void hash_data(FileCopy* fc, MD5* hasher, HL_MD5_CTX* hasherCtx)
{
    /* Hashes binary data */
    hasher->MD5Update(
            hasherCtx,
            fc->_buff.get_processing_byte(),
            fc->_buff.bytesPerBuffer
        );
    fc->_buff.rotate_processing_buffer();
}

std::string get_checksum_from_hasher(MD5* hasher, HL_MD5_CTX* hasherCtx)
{
    /* Generate a checksum from the hashed data */

    std::ostringstream stream;
    unsigned char buff[16] = "";
    hasher->MD5Final(
            (unsigned char*)(buff),
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

        if (!sourceHashInline || fc->_buff.buffers_processed() > 0)
        {
            /* Write the data out to the destination file */
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

    std::cout << "Generating source checksum" << (sourceHashInline ? " (inline)" : "") << "... ";

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
    /* It's, umm... it's main() */
    FileCopy fc;
    
    std::cout << "Starting spatulastic..." << std::endl;

    fc.open_source("../demo_random_data");
    fc.open_dest("demo_random_out");

    execute_transfer(&fc, false);

    std::cout << "Done." << std::endl;
    std::cout << "Spatulastic out!" << std::endl;

    return 0;
}
