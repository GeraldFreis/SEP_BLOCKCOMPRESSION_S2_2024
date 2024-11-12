#include "../Input/BasicInput.h"
#include <cstdio>

int main (int argc, char *argv[]) {
    BasicInput input;
    auto block_sizes = input.get_block_sizes();
    
    return (block_sizes.chunk_x << 16) | (block_sizes.chunk_y << 8) | block_sizes.chunk_z;
}
