#ifndef TREESLINGER_h
#define TREESLINGER_h

#include <sstream>
#include <thread>
#include <atomic>
#include <mutex>
#include <cstring>

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
    FILE_NUM_MISMATCH = 1007,
    CHECKSUM_LIST_LENGTH_MISMATCH = 1008,
};

class TreeSlinger
{
protected:
public:
    bool _sourceHashed, _destHashed, _hashInline;
    int _parentPathLength;
    size_t _size, _transferred;
    std::atomic<int> _sourceQueueIndex;
    std::mutex _sourceQueueLock, _progressLock, _printLock;
    GatherDir _gatherer;
    BasicProgressBar<double> _progress;
    std::vector<FileCopy> _copiers;
    std::vector<std::filesystem::path>* _destFiles;
    std::vector<std::thread> _threads;
    std::vector<std::string> *_sourceChecksums, *_destChecksums;
    
    virtual std::filesystem::path _strip_parent_path(
            std::filesystem::path asset
        );
    virtual std::filesystem::path _get_relative_dest(
            std::filesystem::path sourceAsset
        );
    
    virtual void reset();
    virtual void _create_copiers(int num);
    virtual int _num_copiers();
    virtual void _reset_copiers();

    virtual void _increment_progress(size_t chunk);
    virtual int _get_next_source_index();
    virtual void _sys_file_copy();
    virtual void _run_copier(FileCopy* copier, const size_t totalNumFiles);
    virtual void _spawn_thread(FileCopy* copier);
    
    virtual void _create_dest_dir_structure();
    virtual void _enumerate_dest_files();
    virtual size_t _get_total_size();
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
    
    virtual size_t _copy_file(
            FileCopy* copier,
            std::filesystem::path srcAsset,
            std::filesystem::path destAsset
        );
    
    virtual bool is_complete();

    virtual void _stage();
    virtual bool verify();
    // virtual size_t execute();
    virtual std::vector<std::string>* get_source_checksums() const;
    virtual std::vector<std::string>* get_dest_checksums() const;
};

#endif
