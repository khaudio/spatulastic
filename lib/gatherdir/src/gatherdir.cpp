#include "gatherdir.h"


GatherDir::GatherDir()
{
    this->_paths = new std::vector<std::filesystem::path>();
    this->_directories = new std::vector<std::filesystem::path>();
}

GatherDir::~GatherDir()
{
    delete this->_paths;
    delete this->_directories;
}

void GatherDir::set(std::filesystem::path target)
{
    this->_entry = target;
    for (
            const std::filesystem::directory_entry& entryPoint:
            std::filesystem::recursive_directory_iterator(this->_entry)
        )
    {
        if (entryPoint.is_directory())
        {
            #if _DEBUG
            std::cout << "Found directory " << entryPoint.path() << std::endl;
            #endif
            this->_directories->emplace_back(entryPoint.path());
        }
        else
        {
            #if _DEBUG
            std::cout << "Found asset " << entryPoint.path() << std::endl;
            #endif
            this->_paths->emplace_back(entryPoint.path());
        }
    }
}

void GatherDir::set(std::string target)
{
    set(std::filesystem::path(target));
}

void GatherDir::set(const char* target)
{
    set(std::filesystem::path(target));
}

std::filesystem::path GatherDir::get_entry()
{
    return this->_entry;
}

std::vector<std::filesystem::path>* GatherDir::get_directories()
{
    return this->_directories;
}

std::vector<std::filesystem::path>* GatherDir::get()
{
    return this->_paths;
}

size_t GatherDir::num_directories()
{
    return this->_directories->size();
}

size_t GatherDir::num_files()
{
    return this->_paths->size();
}

void GatherDir::print()
{
    for (const std::filesystem::path& p: *this->_paths)
    {
        std::cout << p.string() << std::endl;
    }
}

void GatherDir::print_directories()
{
    for (const std::filesystem::path& p: *this->_directories)
    {
        std::cout << p.string() << std::endl;
    }
}