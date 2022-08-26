#ifndef SPATULASTIC_H
#define SPATULASTIC_H

#include "treeslinger.h"

void hash_data(
        FileCopy* fc,
        MD5* hasher,
        HL_MD5_CTX* hasherCtx
    );
std::string get_checksum_from_hasher(
        MD5* hasher,
        HL_MD5_CTX* hasherCtx
    );
void execute_transfer(
        FileCopy* fc,
        bool sourceHashInline
    );
int main(int argc, char** argv);

#endif
