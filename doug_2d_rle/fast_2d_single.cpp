#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <immintrin.h>

#define SERIALISER_BUFFER_SIZE 4096

#include "../Input/BasicInput.h"

#ifdef MULTISERIALISER
#include "MultiSerialiser.hpp"
#else
#include "Serialiser.hpp"
#endif

// (x_count + 1) * y_count + 1
#define INPUT_PLANE_SIZE (256 + 1) * 256 + 1

std::atomic_flag using_stdin;

void compress_chunk(
    Input* input,
    Serialiser* serialiser,
    uint32_t* chunk_masks,
    uint8_t tag,
    uint16_t chunk_x,
    uint16_t chunk_y,
    uint16_t z
) {
    uint32_t mask_index = (chunk_y * (256/32)) + (chunk_x / 32);
    for (uint32_t y=0;y < 32; y++) {
        uint16_t line_x = chunk_x;
        uint32_t cur_chunk_mask = chunk_masks[mask_index];
        while (cur_chunk_mask != 0) {
            uint8_t leading_zero = std::countr_zero(cur_chunk_mask);
            cur_chunk_mask >>= leading_zero;
            line_x += leading_zero;

            uint8_t length = std::countr_one(cur_chunk_mask);

            CompressedBlock block;
            block.x_pos = line_x;
            block.y_pos = chunk_y + y;
            block.z_pos = z;
            block.x_size = length;
            block.y_size = 1;
            block.z_size = 1;
            block.tag = tag;

            cur_chunk_mask = (uint32_t)((uint64_t)cur_chunk_mask >> length);

            uint32_t block_mask = (uint32_t)((uint64_t)1 << length) - 1;
            block_mask <<= line_x;

            line_x += length;

            uint32_t next_mask_index = mask_index;

            // check next y row
            while (y + block.y_size < 32) {
                next_mask_index += (256 / 32);
                uint32_t next_block_mask = chunk_masks[next_mask_index] & block_mask;

                if (next_block_mask != (uint32_t)block_mask) break;

                chunk_masks[next_mask_index] &= ~(uint32_t)block_mask;
                block.y_size += 1;
            }

            serialiser->single(input, block);
        }
        mask_index += (256 / 32);
    }
}

int main (int argc, char *argv[]) {
    Serialiser serialiser;
    BasicInput input;
    
    auto tags = input.get_tags_list();
    BlockSizes block_sizes = input.get_block_sizes();
    block_sizes.chunk_x = 32;
    block_sizes.chunk_y = 32;
    block_sizes.count_x = 256;
    block_sizes.count_y = 256;

    char* input_buffer = (char*)malloc(INPUT_PLANE_SIZE);

    uint64_t tag_masks[(256 / 64) * 256];

    for (uint16_t z=0; z<block_sizes.count_z; z++) {
        std::cin.read(input_buffer, INPUT_PLANE_SIZE);

        for (uint8_t tag : tags) {
            char* input_plane = input_buffer;

            __m512i tag_mask = _mm512_set1_epi8(tag);

            // Generate bitmasks
            uint32_t i = 0;
            for (uint16_t y=0; y < 256; y++) {
                for (uint16_t x=0; x < (256/64); x++) {
                    __m512i line = _mm512_loadu_epi8(input_plane);
                    input_plane += 64;

                    tag_masks[i++] = _mm512_cmpeq_epi8_mask(line, tag_mask);
                }
                input_plane += 1;
            }

            // compress each chunk
            for (uint16_t chunk_y=0; chunk_y < 256; chunk_y += 32) {
                for (uint16_t chunk_x=0; chunk_x < 256; chunk_x += 32) {
                    compress_chunk((Input*)&input, &serialiser, (uint32_t*)tag_masks, tag, chunk_x, chunk_y, z);
                }
            }
        }
    }
    serialiser.flush();
}
