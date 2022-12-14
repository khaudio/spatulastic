#include "treeslinger.h"

TreeSlinger::TreeSlinger() :
_sourceHashed(false),
_destHashed(false),
_hashInline(true),
_parentPathLength(0),
_size(0),
_transferred(0),
_sourceQueueIndex(0),
_destQueueIndex(0)
{
    this->_destFiles = new std::vector<std::filesystem::path>();
    this->_sourceChecksums = new std::vector<std::string>();
    this->_destChecksums = new std::vector<std::string>();
}

TreeSlinger::~TreeSlinger()
{
    delete this->_destFiles;
    delete this->_sourceChecksums;
    delete this->_destChecksums;
    // for (FileCopy& copier: this->_copiers)
    // {
    //     copier.close();
    // }
}

void TreeSlinger::reset()
{
    this->source = std::filesystem::path();
    this->destination = std::filesystem::path();
    this->_sourceHashed = false;
    this->_destHashed = false;
    this->_parentPathLength = 0;
    this->_sourceQueueIndex = 0;
    this->_destQueueIndex = 0;
    this->_size = 0;
    this->_transferred = 0;
    delete this->_destFiles;
    this->_destFiles = new std::vector<std::filesystem::path>();
    _reset_copiers();
    this->_threads.erase(this->_threads.begin(), this->_threads.end());
    this->_sourceHasherThreads.erase(this->_threads.begin(), this->_threads.end());
    this->_destHasherThreads.erase(this->_threads.begin(), this->_threads.end());
    this->_progress.set_maximum(0);
    this->_progress.set(0);
    this->_progress.set_chunk_size(0);
    size_t checksumListLength(this->_sourceChecksums->size());
    #if _DEBUG
    if (this->_destChecksums->size() != checksumListLength)
    {
        throw CHECKSUM_LIST_LENGTH_MISMATCH;
    }
    #endif
    for (size_t i(0); i < checksumListLength; ++i)
    {
        this->_sourceChecksums->at(i) = std::string();
        this->_destChecksums->at(i) = std::string();
    }
    
    /*
    - reset progress
        - build in ::reset()
    - reset gatherer
    - kill threads with atomic bool kill flag or something
    */

}

std::filesystem::path TreeSlinger::relative_dest(
        std::filesystem::path srcDir,
        std::filesystem::path destDir
    )
{
    std::string srcString(srcDir.string());
    int parentLength = srcDir.parent_path().string().size();
    
    #if _DEBUG
    std::string check;
    std::string parentString(srcDir.parent_path().string());
    std::string relative = srcString.substr(
            parentLength,
            srcString.size() - parentLength
        );
    int srcLength(srcString.size());
    for (int i(0); i < srcLength; ++i)
    {
        if (parentString[i] != srcString[i])
        {
            check = srcString.substr(i, srcLength - i);
            break;
        }
    }
    if (check != relative)
    {
        throw STRING_MISMATCH;
    }
    #endif

    destDir += std::filesystem::path(srcString.substr(
            parentLength,
            srcString.size() - parentLength
        ));
    return std::filesystem::canonical(destDir);
}

inline std::filesystem::path TreeSlinger::_strip_parent_path(
        std::filesystem::path asset
    )
{
    #if _DEBUG
    if (!this->_parentPathLength)
    {
        throw PARENT_PATH_NOT_SET;
    }
    #endif

    return std::filesystem::path(asset.string().substr(
            this->_parentPathLength,
            asset.string().size()
        ));
}

inline std::filesystem::path TreeSlinger::_get_relative_dest(
        std::filesystem::path sourceAsset
    )
{
    #if _DEBUG
    if (!this->_parentPathLength)
    {
        throw PARENT_PATH_NOT_SET;
    }
    if (this->destination.empty())
    {
        throw DEST_DIR_NOT_SET;
    }
    #endif
    
    std::string redirected(this->destination.string());
    redirected += sourceAsset.string().substr(
            this->_parentPathLength,
            sourceAsset.string().size()
        );
    return std::filesystem::path(redirected).lexically_normal();
}

void TreeSlinger::_create_copiers(int num)
{
    #if _DEBUG
    if (num < 1)
    {
        throw std::out_of_range("Num copiers must be >= 1");
    }
    #endif
    this->_threads.reserve(num);
    this->_copiers.reserve(num);
    for (int i(0); i < num; ++i)
    {
        this->_copiers.emplace_back(FileCopy());
    }
}

inline int TreeSlinger::_num_copiers()
{
    return this->_copiers.size();
}

void TreeSlinger::_reset_copiers()
{
    for (int i(0); i < _num_copiers(); ++i)
    {
        this->_copiers[i].reset();
    }
}

int TreeSlinger::_get_next_source_index()
{
    const std::lock_guard<std::mutex> lock(this->_sourceQueueLock);
    return this->_sourceQueueIndex++;
}

int TreeSlinger::_get_next_dest_index()
{
    const std::lock_guard<std::mutex> lock(this->_destQueueLock);
    return this->_destQueueIndex++;
}

void TreeSlinger::_increment_progress(size_t chunk)
{
    const std::lock_guard<std::mutex> lock(this->_progressLock);
    this->_progress.increment(chunk);
}

void TreeSlinger::_sys_file_copy()
{
    /* Recursive file copy
    Throws std::filesystem::filesystem_error if file exists */
    std::filesystem::copy(
            this->source,
            this->destination,
            std::filesystem::copy_options::recursive
        );
}

void TreeSlinger::_run_copier(FileCopy* copier, const size_t totalNumFiles)
{
    /* Runs a single FileCopy object until all files are copied */
    size_t bytesCopied, index(_get_next_source_index());
    std::vector<std::filesystem::path>* sources = this->_gatherer.get();
    while (index < totalNumFiles)
    {
        copier->reset();
        std::this_thread::yield();
        copier->open_source(sources->at(index));
        std::this_thread::yield();
        copier->open_dest(this->_destFiles->at(index));
        std::this_thread::yield();
        bytesCopied = copier->execute();
        _increment_progress(bytesCopied);
        std::this_thread::yield();
        index = _get_next_source_index();
    }
}

void TreeSlinger::_spawn_thread(FileCopy* copier)
{
    this->_threads.emplace_back(std::thread(
            &TreeSlinger::_run_copier,
            this,
            copier,
            this->_gatherer.num_files()
        ));
}

void TreeSlinger::_run_source_hasher(const size_t totalNumFiles)
{
    size_t index(_get_next_source_index());
    md5wrapper hasher;
    std::string checksum;
    std::vector<std::filesystem::path>* sources = this->_gatherer.get();
    while (index < totalNumFiles)
    {
        this->_sourceChecksums->at(index) = hasher.getHashFromFile(
                sources->at(index).string()
            );
        std::this_thread::yield();
        index = _get_next_source_index();
        std::this_thread::yield();
    }
}

void TreeSlinger::_run_dest_hasher(const size_t totalNumFiles)
{
    size_t index(_get_next_dest_index());
    md5wrapper hasher;
    std::string checksum;
    while (index < totalNumFiles)
    {
        this->_destChecksums->at(index) = hasher.getHashFromFile(
                this->_destFiles->at(index).string()
            );
        std::this_thread::yield();
        index = _get_next_dest_index();
        std::this_thread::yield();
    }
}

void TreeSlinger::_spawn_source_hasher_thread()
{
    this->_sourceHasherThreads.emplace_back(std::thread(
            &TreeSlinger::_run_source_hasher,
            this,
            this->_gatherer.num_files()
        ));
}

void TreeSlinger::_spawn_dest_hasher_thread()
{
    this->_destHasherThreads.emplace_back(std::thread(
            &TreeSlinger::_run_dest_hasher,
            this,
            this->_gatherer.num_files()
        ));
}

void TreeSlinger::_create_dest_dir_structure()
{
    #if _DEBUG
    if (this->source.empty()) throw SOURCE_NOT_SET;
    if (this->destination.empty()) throw DEST_NOT_SET;
    #endif
    for (const std::filesystem::path p: *(this->_gatherer.get_directories()))
    {
        std::filesystem::path redirected(_get_relative_dest(p));
        std::filesystem::create_directories(redirected);
        #if _DEBUG
        std::cout << "Created directory " << redirected << std::endl;
        #endif
    }
}

void TreeSlinger::_enumerate_dest_files()
{
    for (const std::filesystem::path& p: *(this->_gatherer.get()))
    {
        std::filesystem::path redirected(_get_relative_dest(p));
        this->_destFiles->emplace_back(redirected);
    }
}

size_t TreeSlinger::_get_total_size()
{
    this->_size = 0;
    for (const std::filesystem::path& p: *(this->_gatherer.get()))
    {
        this->_size += std::filesystem::file_size(p);
    }
    return this->_size;
}

void TreeSlinger::_allocate_checksums()
{
    #if _DEBUG
    if (_checksums_allocated())
    {
        std::cout << "Checksum lists already allocated; returning";
        return;
    }
    #endif
    size_t numFiles(this->_gatherer.num_files());
    this->_sourceChecksums->reserve(numFiles);
    this->_destChecksums->reserve(numFiles);
    for (size_t i(0); i < numFiles; ++i)
    {
        this->_sourceChecksums->emplace_back(std::string());
        this->_destChecksums->emplace_back(std::string());
    }
}

bool TreeSlinger::_checksums_allocated()
{
    return (
            (this->_sourceChecksums->size() > 0)
            && (this->_destChecksums->size() > 0)
        );

    return true;
}

void TreeSlinger::_hash_source()
{
    #if _DEBUG
    if (!_checksums_allocated()) throw CHECKSUM_CONTAINER_IS_NULL;
    #endif
    if (this->_sourceHashed) return;
    
    /*
        hash source here
    */
    
}

void TreeSlinger::_hash_dest()
{
    #if _DEBUG
    if (!_checksums_allocated()) throw CHECKSUM_CONTAINER_IS_NULL;
    #endif
    if (this->_destHashed) return;
    
    /*
        hash dest here
    */
    
}

void TreeSlinger::_create_csv()
{
    /* #if _DEBUG
    if (this->_sourceChecksums->empty()) throw SOURCE_NOT_HASHED;
    if (this->_destChecksums->empty()) throw DEST_NOT_HASHED;
    #endif */

    std::ofstream report;
    std::stringstream filename;
    std::wstring wseparator;
    wseparator += std::filesystem::path::preferred_separator;
    std::string separator(wseparator.begin(), wseparator.end());
    size_t numFiles(this->_gatherer.num_files());

    filename << this->destination.string();
    filename << separator;
    filename << "today_now_"; // Insert date and time

    #if _DEBUG
    std::cout << "Creating csv " << filename.str();
    std::cout << std::endl;
    #endif

    report.open(filename.str(), std::ofstream::out);
    report << "Filename," << this->algorithm << " Checksum" << std::endl;

    for (size_t i(0); i < numFiles; ++i)
    {
        report << this->_destFiles->at(i);
        report << ",";
        // report << this->_destChecksums->at(i);
        
        report << "placeholder_checksum";
        
        report << std::endl;
    }
}

void TreeSlinger::set_source(std::filesystem::path sourcePath)
{
    this->source = std::filesystem::canonical(sourcePath);
    this->_parentPathLength = this->source.parent_path().string().size();
    this->_gatherer.set(this->source);
    this->_progress.set_maximum(_get_total_size());
    this->_progress.set(size_t(0));
    this->_destFiles->reserve(this->_gatherer.num_files());
}

void TreeSlinger::set_destination(std::filesystem::path destPath)
{
    this->destination = std::filesystem::canonical(destPath);
}

void TreeSlinger::set_hash_algorithm(const char* algo)
{
    this->algorithm = algo;
}

void TreeSlinger::set_hash_inline(bool hashInline)
{
    this->_hashInline = true;
}

std::vector<std::filesystem::path>* TreeSlinger::get_source_files()
{
    return this->_gatherer.get();
}

std::vector<std::filesystem::path>* TreeSlinger::get_dest_files()
{
    return this->_destFiles;
}

size_t TreeSlinger::_copy_file(
        FileCopy* copier,
        std::filesystem::path srcAsset,
        std::filesystem::path destAsset
    )
{
    #if _DEBUG
    std::cout << "Resetting copier" << std::endl;
    #endif

    copier->reset();

    #if _DEBUG
    std::cout << "Opening source: ";
    std::cout << srcAsset.string() << std::endl;
    #endif

    copier->open_source(srcAsset);

    #if _DEBUG
    std::cout << "Opening dest: ";
    std::cout << destAsset.string() << std::endl;
    #endif

    copier->open_dest(destAsset);

    #if _DEBUG
    std::cout << "Copier is ";
    std::cout << (copier->ready() ? "" : "not ");
    std::cout << "ready" << std::endl;
    #endif

    #if _DEBUG
    std::cout << "Executing..." << std::endl;
    #endif

    size_t numBytes = copier->execute();
    this->_transferred += numBytes;
    this->_progress.set(this->_transferred);
    
    #if _DEBUG
    std::cout << "Execute completed; transferred ";
    std::cout << this->_transferred << " bytes" << std::endl;
    #endif

    return this->_transferred;
}

bool TreeSlinger::is_complete()
{
    return this->_progress.is_complete();
}

void TreeSlinger::_stage()
{
    _create_dest_dir_structure();
    _enumerate_dest_files();
    _allocate_checksums();
}

bool TreeSlinger::verify()
{
    #if _DEBUG
    if (this->_gatherer.num_files() != this->_destFiles->size())
    {
        throw FILE_NUM_MISMATCH;
    }
    std::cout << "Verifying..." << std::endl;
    #endif

    md5wrapper hasher;
    size_t index(0), totalNumFiles = this->_gatherer.num_files();
    std::vector<std::filesystem::path>* sources = this->_gatherer.get();
    while (index < totalNumFiles)
    {
        #if _DEBUG
        std::cout << "Hashing file ";
        std::cout << sources->at(index).string() << std::endl;
        #endif
        this->_sourceChecksums->at(index) = hasher.getHashFromFile(
                sources->at(index).string()
            );
        ++index;
    }

    index = 0;
    while (index < totalNumFiles)
    {
        #if _DEBUG
        std::cout << "Hashing file ";
        std::cout << this->_destFiles->at(index).string() << std::endl;
        #endif
        this->_destChecksums->at(index) = hasher.getHashFromFile(
                this->_destFiles->at(index).string()
            );
        ++index;
    }
    
    index = 0;
    while (index < totalNumFiles)
    {
        if (
                this->_sourceChecksums->at(index)
                != this->_destChecksums->at(index)
            )
        {
            #if _DEBUG
            std::cerr << "Checkum mismatch for files\n";
            std::cerr << sources->at(index).string();
            std::cerr << "\n\t" << this->_sourceChecksums->at(index);
            std::cerr << "\nand\n";
            std::cerr << this->_destFiles->at(index).string();
            std::cerr << "\n\t" << this->_destChecksums->at(index);
            std::cerr << std::endl;
            throw CHECKSUM_MISMATCH;
            #endif
            return false;
        }
        ++index;
    }

    #if _DEBUG
    std::cout << "Verified" << std::endl;
    #endif
    
    return true;
}

bool TreeSlinger::verify_threaded(int numThreads)
{
    #if _DEBUG
    if (this->_gatherer.num_files() != this->_destFiles->size())
    {
        throw FILE_NUM_MISMATCH;
    }
    std::cout << "Verifying..." << std::endl;
    #endif

    this->_sourceQueueIndex = 0;
    this->_destQueueIndex = 0;
    for (int i(0); i < numThreads; ++i)
    {
        _spawn_source_hasher_thread();
        _spawn_dest_hasher_thread();
    }

    for (int i(0); i < numThreads; ++i)
    {
        this->_sourceHasherThreads[i].join();
        this->_destHasherThreads[i].join();
    }
    
    size_t index(0), totalNumFiles = this->_gatherer.num_files();
    while (index < totalNumFiles)
    {
        if (
                this->_sourceChecksums->at(index)
                != this->_destChecksums->at(index)
            )
        {
            #if _DEBUG
            std::vector<std::filesystem::path>* sources = this->_gatherer.get();
            std::cerr << "Checkum mismatch for files\n";
            std::cerr << sources->at(index).string();
            std::cerr << "\n\t" << this->_sourceChecksums->at(index);
            std::cerr << "\nand\n";
            std::cerr << this->_destFiles->at(index).string();
            std::cerr << "\n\t" << this->_destChecksums->at(index);
            std::cerr << std::endl;
            throw CHECKSUM_MISMATCH;
            #endif
            return false;
        }
        ++index;
    }

    #if _DEBUG
    std::cout << "Verified" << std::endl;
    #endif
    
    return true;
}


// size_t TreeSlinger::execute()
// {

// }

std::vector<std::string>* TreeSlinger::get_source_checksums() const
{
    return this->_sourceChecksums;
}

std::vector<std::string>* TreeSlinger::get_dest_checksums() const
{
    return this->_destChecksums;
}
