#include "filecopy.h"
#include "progressbar.h"
#include "ringbuffer.h"


void execute_transfer(FileCopy* fc)
{
    /* Execute transfer with a progress bar */

    BasicProgressBar<double> bar;
    bar.set_maximum(fc->get_source_size());
    char status[40];

    fc->read_to_buffer();
    while (!fc->complete())
    {
        fc->read_to_buffer();
        size_t bytesMoved = fc->write_from_buffer();
        bar.increment(bytesMoved);
        bar.print(status, sizeof(status));
        for (size_t i(0); i < sizeof(status); ++i)
        {
            std::cout << status[i];
        }
        std::cout << std::setw(2) << " " << fc->bytes_remaining();
        std::cout << " of " << fc->get_source_size() << " bytes remaining";
        std::cout << "\r";
    }

    fc->close();

    std::cout << std::endl;
}


int main(int argc, char** argv)
{
    FileCopy fc;
    
    std::cout << "Starting spatulastic..." << std::endl;

    fc.open_source("../demodata.txt");
    fc.open_dest("demo_write_out.txt");

    execute_transfer(&fc);

    std::cout << "Done." << std::endl;
    std::cout << "Spatulastic out!" << std::endl;

    return 0;
}
