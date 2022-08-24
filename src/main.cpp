// #include "cmakeconfig.h"
#include "treeslinger.h"

/* Whether to hash the source file
while it's being moved in the ring buffer */
#define INLINEHASH                  false


void hash_data(FileCopy* fc, MD5* hasher);
std::string get_checksum_from_hasher(MD5* hasher, HL_MD5_CTX* hasherCtx);
void execute_transfer(FileCopy* fc);
int main(int argc, char** argv);


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
    char status[41];
    size_t progressBarWidth = sizeof(status) - 1;

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
        bar.get_bar(status, progressBarWidth);
        for (size_t i(0); i < progressBarWidth; ++i)
        {
            std::cout << status[i];
        }
        std::cout << "\r";
    }
    std::cout << std::endl;

    std::cout << "Closing file..." << std::endl;

    /* Close the files */
    fc->close();

    fc->_destSizeInBytes = std::filesystem::file_size(fc->dest);
    if (fc->_sourceSizeInBytes != fc->_destSizeInBytes)
    {
        throw FILESIZE_MISMATCH;
    }
    if (fc->_numBytesReadToBuffer != fc->_numBytesWrittenFromBuffer)
    {
        throw READ_WRITE_MISMATCH;
    }

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
        sourceHash = sourceHashWrapper.getHashFromFile(fc->source.string());
    }
    
    std::cout << "Done." << std::endl;
    std::cout << "Generating destination checksum... ";

    /* Get destination file checksum now that the transfer is complete */
    destHash = destHashWrapper.getHashFromFile(fc->dest.string());

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
    std::cout << "Starting spatulastic..." << std::endl;
    #if _DEBUG
    std::cout << "Debug mode enabled" << std::endl;
    #endif

    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::high_resolution_clock::duration duration;
    start = std::chrono::high_resolution_clock::now();

    std::cout << "Running..." << std::endl;

    TreeSlinger t;
    std::filesystem::path src("../example_data");
    std::filesystem::path dest("../build");
    BasicProgressBar<double> bar;
    char status[41];
    size_t progressBarWidth = sizeof(status) - 1;
    
    std::cout << "running t.set_source(src);" << std::endl;
    t.set_source(src);
    std::cout << "running t.set_destination(dest);" << std::endl;
    t.set_destination(dest);
    std::cout << "running t.set_hash_algorithm('md5');" << std::endl;
    t.set_hash_algorithm("md5");
    std::cout << "running t._create_copiers(1);" << std::endl;
    t._create_copiers(1);
    std::cout << "running t._create_dest_dir_structure();" << std::endl;
    t._create_dest_dir_structure();
    std::cout << "running t._enumerate_dest_files();" << std::endl;
    t._enumerate_dest_files();
    std::cout << "running t._allocate_checksums();" << std::endl;
    t._allocate_checksums();
    std::cout << "running t.set_hash_inline(INLINEHASH);" << std::endl;
    t.set_hash_inline(INLINEHASH);
    std::cout << "running size_t totalSize = t._get_total_size();" << std::endl;
    size_t totalSize = t._get_total_size();
    std::cout << "total size is " << totalSize << " bytes" << std::endl;
    std::cout << "running bar.set_maximum(totalSize);" << std::endl;
    bar.set_maximum(totalSize);
    std::cout << "running size_t numFiles(t._gatherer.num_files());" << std::endl;
    size_t numFiles(t._gatherer.num_files());
    std::cout << "running std::vector<std::filesystem::path>* sources(t._gatherer.get());" << std::endl;
    std::vector<std::filesystem::path>* sources(t._gatherer.get());

    t._run_copier(&(t._copiers[0]));

    std::cout << "running t._create_csv();" << std::endl;
    t._create_csv();
    std::cout << "Transfer complete" << std::endl;

    end = std::chrono::high_resolution_clock::now();
    duration = end - start;
    std::cout << "Transfer took ";
    std::cout << std::fixed << std::setprecision(2);
    std::cout << static_cast<double>(duration.count()) / 1000000;
    std::cout << " ms" << std::endl;
    std::ofstream log;
    log.open("log.txt", std::ios::app);
    log << "Source hashed " << (INLINEHASH ? "inline" : "after ") << "\t";
    log << (static_cast<double>(duration.count()) / 1000000);
    log  << " ms" << std::endl;
    log.close();
    std::cout << "Spatulastic out!" << std::endl;

    return 0;
}
