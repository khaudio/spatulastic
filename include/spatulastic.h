#ifndef SPATULASTIC_H
#define SPATULASTIC_H

#include "cmakeconfig.h"

void hash_data(FileCopy* fc, MD5* hasher);
std::string get_checksum_from_hasher(MD5* hasher, HL_MD5_CTX* hasherCtx, bool sourceHashInline = false);
void execute_transfer(FileCopy* fc);
int main(int argc, char** argv);

#endif
