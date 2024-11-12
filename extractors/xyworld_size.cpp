#include "../Input/BasicInput.h"

int main (int argc, char *argv[]) {
    BasicInput input;
    auto block_sizes = input.get_block_sizes();
    
    return (block_sizes.count_y << 16) | block_sizes.count_x;
}
