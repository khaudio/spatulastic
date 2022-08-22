#ifndef TREESLINGER_h
#define TREESLINGER_h

#include <sstream>

#include "filecopy.h"
#include "gatherdir.h"
#include "progressbar.h"
#include "ringbuffer.h"
#include "hashlibpp.h"


enum treeslinger_err
{
    CHECKSUM_CONTAINER_IS_NULL = 1001,
    CHECKSUM_MISMATCH = 1002,
    STRING_MISMATCH = 1003,
    PARENT_PATH_NOT_SET = 1004,
    SOURCE_NOT_HASHED = 1005,
    DEST_NOT_HASHED = 1006,
};

class TreeSlinger
{
protected:
public:
    bool _sourceHashed, _destHashed, _hashInline;
    int _parentPathLength;
    GatherDir _gatherer;
    std::vector<FileCopy> _copiers;
    std::vector<std::filesystem::path>* _destFiles;
    std::vector<char[16]> *_sourceChecksums, *_destChecksums;
    
    virtual std::filesystem::path _strip_parent_path(
            std::filesystem::path asset
        );
    virtual std::filesystem::path _get_relative_dest(
            std::filesystem::path sourceAsset
        );
    
    virtual void _create_copiers(int num);
    virtual int _num_copiers();
    virtual void _reset_copiers();
    
    virtual void _create_dest_dir_structure();
    virtual void _enumerate_dest_files();
    virtual void _allocate_checksums();
    virtual bool _checksums_allocated();
    
    virtual void _hash_source();
    virtual void _hash_dest();
    
    virtual void _create_csv();
    
public:
    std::filesystem::path source, destination;
    std::string algorithm;

    TreeSlinger();
    ~TreeSlinger();
    
    static std::filesystem::path relative_dest(
            std::filesystem::path srcDir,
            std::filesystem::path destDir
        );

    virtual void set_source(std::filesystem::path sourcePath);
    virtual void set_destination(std::filesystem::path destPath);
    virtual void set_hash_algorithm(const char* algo);
    virtual void set_hash_inline(bool hashInline = true);
    
    // virtual int execute();
    // virtual bool verify();
    // virtual std::vector<char[16]>* get_checksums();
};

#endif
