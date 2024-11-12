#include <immintrin.h>

#include "../Input/BasicInput.h"
#include "Serialiser.h"

// longest x is 700
uint8_t mod14table[700];

#define CBLOCK_BUFF_SIZE 200

int main (int argc, char *argv[]) {
    Serialiser serialiser;
    BasicInput input;

    uint8_t counter = 0;
    for (int i = 0; i < 700; i++) {
        if (counter == 14) {
            counter = 0;
        }
        mod14table[i] = counter++;
    }

    BlockSizes block_sizes = input.get_block_sizes();
    auto tags = input.get_tags_list();

    char* input_line = (char*)malloc(block_sizes.count_x + block_sizes.chunk_x);

    uint32_t block_count = 0;
    CompressedBlock* blocks = (CompressedBlock*)malloc(CBLOCK_BUFF_SIZE * sizeof(CompressedBlock));

    uint64_t simd_width = 64 - (64 % block_sizes.chunk_x);
    uint64_t simd_width_mask = ((uint64_t)1 << simd_width) - 1;

    for (uint16_t z=0; z < block_sizes.count_z; z++) {
        for (uint16_t y=0; y < block_sizes.count_y; y++) {
            std::cin.read(input_line, block_sizes.count_x + 1);

            uint16_t x = 0;
            while (x < block_sizes.count_x) {

                __m512i line = _mm512_loadu_epi8((const void*)(input_line + x));

                for (uint8_t tag : tags) {

                    __m512i tag_mask = _mm512_set1_epi8(tag);

                    uint64_t cmp_mask = _mm512_mask_cmpeq_epi8_mask(simd_width_mask, line, tag_mask);

                    uint16_t line_x = x;
                    while (cmp_mask != 0) {
                        // find start of block
                        uint16_t leading_zero = std::__countr_zero(cmp_mask);
                        cmp_mask >>= leading_zero;
                        line_x += leading_zero;

                        // get length
                        uint64_t parent_mask = (1 << (block_sizes.chunk_x - mod14table[line_x])) - 1;
                        uint16_t length = std::__countr_one(cmp_mask & parent_mask);
                        
                        CompressedBlock* block = &blocks[block_count++];
                        block->x_pos = line_x;
                        block->y_pos = y;
                        block->z_pos = z;
                        block->x_size = length;
                        block->y_size = 1;
                        block->z_size = 1;
                        block->tag = tag;

                        cmp_mask >>= length;
                        line_x += length;

                        if (block_count >= CBLOCK_BUFF_SIZE) {
                            serialiser.serialise(&input, blocks, block_count);
                            block_count = 0;
                        }

                        // serialiser.single(&input, *block);
                    }
                }

                x += simd_width;
            }
        }
        std::cin.read(input_line, 1);
    }

    serialiser.serialise(&input, blocks, block_count);
    serialiser.flush();

    return 0;
}
