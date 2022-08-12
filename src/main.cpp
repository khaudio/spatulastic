#include "filecopy.h"
#include "progressbar.h"
#include "ringbuffer.h"

int main(int argc, char** argv)
{
    FileCopy fm;
    
    fm.open_source("../demodata.txt");
    fm.open_dest("demo_write_out.txt");
    
    fm.execute();
    
    return 0;
}
