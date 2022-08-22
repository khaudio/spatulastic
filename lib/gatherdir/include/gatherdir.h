#ifndef GATHERDIR_H
#define GATHERDIR_H

#include <filesystem>
#include <iostream>
#include <vector>

class GatherDir
{
protected:
    std::filesystem::path _entry;
    std::vector<std::filesystem::path> *_directories, *_paths;

public:
    GatherDir();
    ~GatherDir();

    virtual void set(std::filesystem::path target);
    virtual void set(const char* target);
    virtual void set(std::string target);
    
    virtual std::filesystem::path get_entry();
    virtual std::vector<std::filesystem::path>* get_directories();
    virtual std::vector<std::filesystem::path>* get();
    
    size_t num_directories();
    size_t num_files();
    
    virtual void print();
    virtual void print_directories();
};

#endif
