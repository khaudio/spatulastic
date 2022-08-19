#include "gatherdir.h"


GatherDir::GatherDir()
{
    this->paths = new std::vector<std::filesystem::path>();
    this->directories = new std::vector<std::filesystem::path>();
}

GatherDir::~GatherDir()
{
    delete this->paths;
    delete this->directories;
}

void GatherDir::set(std::filesystem::path target)
{
    this->entry = target;
    for (
            const std::filesystem::directory_entry& entryPoint:
            std::filesystem::recursive_directory_iterator(target)
        )
    {
        if (entryPoint.is_directory())
        {
            this->directories->emplace_back(entryPoint.path());
        }
        this->paths->emplace_back(entryPoint.path());
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
    return this->entry;
}

std::vector<std::filesystem::path>* GatherDir::get_directories()
{
    return this->directories;
}

std::vector<std::filesystem::path>* GatherDir::get()
{
    return this->paths;
}

void GatherDir::print()
{
    for (const std::filesystem::path& p: *this->paths)
    {
        std::cout << p.string() << std::endl;
    }
}

void GatherDir::print_directories()
{
    for (const std::filesystem::path& p: *this->directories)
    {
        std::cout << p.string() << std::endl;
    }
}