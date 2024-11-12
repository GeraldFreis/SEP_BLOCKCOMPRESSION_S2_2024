#include "../../Input/BasicInput.h"
#include "../../Input/FirstInput.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf("specify which input to use [f/b]\n");
        return 1;
    }
    Input* input = nullptr;
    switch (argv[1][0]) {
        case 'f':
            input = new FirstInput;
        break;
        case 'b':
            input = new BasicInput;
        break;
        default: {
            printf("invalid\n");
            return -1;
        }
    }

    BlockSizes block_sizes = input->get_block_sizes();

    // print first line
    printf("%zu,%zu,%zu,%zu,%zu,%zu\n",
           block_sizes.count_x,
           block_sizes.count_y,
           block_sizes.count_z,
           block_sizes.chunk_x,
           block_sizes.chunk_y,
           block_sizes.chunk_z);

    // print tags
    auto tags_list = input->get_tags_list();
    for (uint8_t tag : tags_list) {
        printf("%c, %s\n", (char)tag, input->get_tag_label(tag));
    }
    printf("\n");

    uint8_t* chunk = (uint8_t*)malloc(block_sizes.count_x * block_sizes.count_y * block_sizes.chunk_z);

    for (size_t i = 0; i < block_sizes.count_z / block_sizes.chunk_z; i++) {
        input->get_z_layer(chunk);
        size_t count = 0;
        for (size_t z = 0; z < block_sizes.chunk_z; z++) {
            for (size_t y = 0; y < block_sizes.count_y; y++) {
                std::cout.write((char*)chunk + count, block_sizes.count_x);
                count += block_sizes.count_x;
                printf("\n");
            }
            printf("\n");
        }
    }
}
