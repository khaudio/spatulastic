#include "treeslinger.h"

TreeSlinger::TreeSlinger() :
_sourceHashed(false),
_destHashed(false),
_hashInline(true),
_parentPathLength(0)
{
    this->_destFiles = new std::vector<std::filesystem::path>();
    this->_sourceChecksums = new std::vector<char[16]>();
    this->_destChecksums = new std::vector<char[16]>();
}

TreeSlinger::~TreeSlinger()
{
    delete this->_sourceChecksums;
    delete this->_destChecksums;
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

    return destDir += std::filesystem::path(srcString.substr(
            parentLength,
            srcString.size() - parentLength
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
    return redirected += sourceAsset.string().substr(
            this->_parentPathLength,
            sourceAsset.string().size()
        );
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
    for (const std::filesystem::path p: *(this->_gatherer.get_directories()))
    {
        std::filesystem::path redirected(_get_relative_dest(p));
        /* std::filesystem::create_directory(redirected, p); */
        std::filesystem::create_directories(redirected);
    }
}

void TreeSlinger::_enumerate_dest_files()
{
    for (const std::filesystem::path p: *(this->_gatherer.get()))
    {
        std::filesystem::path redirected(_get_relative_dest(p));
        this->_destFiles->emplace_back(redirected);
    }
}

void TreeSlinger::_allocate_checksums()
{
    size_t numFiles = this->_gatherer.num_files();
    for (int i(0); i < numFiles; ++i)
    {
        /* char srcBuff[16];
        char destBuff[16];
        this->_sourceChecksums->emplace_back(srcBuff);
        this->_destChecksums->emplace_back(destBuff); */
    }
}

bool TreeSlinger::_checksums_allocated()
{
    return (
            (this->_sourceChecksums->size() > 0)
            && (this->_destChecksums->size() > 0)
        );
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

        make dest path absolute?
        ../build in csv is not all that helpful, especially when
        the report is IN "build"; i.e., relative path is incorrect
        when viewed in the csv
    */


    // #if _DEBUG
    // if (this->_sourceChecksums->empty()) throw SOURCE_NOT_HASHED;
    // if (this->_destChecksums->empty()) throw DEST_NOT_HASHED;
    // #endif

    std::ofstream report;
    std::wstringstream filename;
    size_t numFiles(this->_gatherer.num_files());

    filename << this->destination.wstring();
    filename << std::filesystem::path::preferred_separator;
    filename << "today_now_"; // Insert date and time

    #if _DEBUG
    std::cout << "Creating csv ";
    std::wstring wstr(filename.str());
    std::cout << std::string(wstr.begin(), wstr.end());
    std::cout << std::endl;
    #endif

    report.open(filename.str(), std::ofstream::out);
    report << "Filename," << this->algorithm << " Checksum" << std::endl;

    for (int i(0); i < numFiles; ++i)
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
    this->source = sourcePath;
    this->_parentPathLength = this->source.parent_path().string().size();
    this->_gatherer.set(this->source);
    this->_destFiles->reserve(this->_gatherer.num_files());
}

void TreeSlinger::set_destination(std::filesystem::path destPath)
{
    this->destination = destPath;
}

void TreeSlinger::set_hash_algorithm(const char* algo)
{
    this->algorithm = algo;
}

void TreeSlinger::set_hash_inline(bool hashInline)
{
    this->_hashInline = true;
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
