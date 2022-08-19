#ifndef GATHERDIR_H
#define GATHERDIR_H

#include <filesystem>
#include <iostream>
#include <vector>

class GatherDir
{
protected:
    std::filesystem::path entry;
    std::vector<std::filesystem::path> *directories, *paths;

public:
    GatherDir();
    ~GatherDir();

    void set(std::filesystem::path target);
    void set(const char* target);
    void set(std::string target);
    std::filesystem::path get_entry();
    std::vector<std::filesystem::path>* get_directories();
    std::vector<std::filesystem::path>* get();
    void print();
    void print_directories();
};

#endif
