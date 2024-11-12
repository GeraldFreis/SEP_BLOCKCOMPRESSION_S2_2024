#include "CompressionFixedSize.h"
#include <iostream>

int main () {
    /* test with large block size == parent block size */
    CompressionFixedSize compression;

    compression.Compress();

    return 0;
}