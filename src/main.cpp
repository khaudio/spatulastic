#include "filecopy.h"
#include "progressbar.h"
#include "ringbuffer.h"


int main(int argc, char** argv)
{
    FileCopy fm;
    
    std::cout << "Starting spatulastic..." << std::endl;

    fm.open_source("../demodata.txt");
    fm.open_dest("demo_write_out.txt");

    fm.execute();
    
    std::cout << "Done." << std::endl;
    std::cout << "Spatulastic out!" << std::endl;

    return 0;
}
