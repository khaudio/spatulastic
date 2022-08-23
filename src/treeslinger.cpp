#include "treeslinger.h"

TreeSlinger::TreeSlinger() :
_sourceHashed(false),
_destHashed(false),
_hashInline(true),
_parentPathLength(0),
_size(0),
_transferred(0)
{
    this->_destFiles = new std::vector<std::filesystem::path>();
    // this->_sourceChecksums = new std::vector<char[16]>();
    // this->_destChecksums = new std::vector<char[16]>();
}

TreeSlinger::~TreeSlinger()
{
    // delete this->_sourceChecksums;
    // delete this->_destChecksums;
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

void TreeSlinger::_create_dest_dir_structure()
{
    #if _DEBUG
    std::cout << "Redirecting directories " << std::endl;
    #endif
    for (const std::filesystem::path p: *(this->_gatherer.get_directories()))
    {
        #if _DEBUG
        std::cout << "Redirecting directory " << p << " to destination" << std::endl;
        #endif
        std::filesystem::path redirected(_get_relative_dest(p));
        #if _DEBUG
        std::cout << "Creating directory " << redirected << std::endl;
        #endif
        /* std::filesystem::create_directory(redirected, p); */
        std::filesystem::create_directories(redirected);
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
    size_t numFiles = this->_gatherer.num_files();
    for (size_t i(0); i < numFiles; ++i)
    {
        /* char srcBuff[16];
        char destBuff[16];
        this->_sourceChecksums->emplace_back(srcBuff);
        this->_destChecksums->emplace_back(destBuff); */
    }
}

bool TreeSlinger::_checksums_allocated()
{
    // return (
    //         (this->_sourceChecksums->size() > 0)
    //         && (this->_destChecksums->size() > 0)
    //     );

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


    /*
        is the file and hash ordering really consistent here... ????

        add actual date and time stamp
    */


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

void TreeSlinger::_serve_files()
{
    while (1)
    {
        
    }
}

// std::filesystem::path TreeSlinger::_get_next_file()
// {
    
// }

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
    
    #if _DEBUG
    std::cout << "Execute completed; transferred ";
    std::cout << this->_transferred << " bytes" << std::endl;
    #endif

    return this->_transferred;
}

// int TreeSlinger::execute()
// {
//     _create_dest_dir_structure();
// }

// bool TreeSlinger::verify()
// {
//     /* check that num dest files == num source files, maybe a counter;
//     check that all transfers are complete
//     */
// }

// std::vector<char[16]>* TreeSlinger::get_checksums()
// {
    
// }
