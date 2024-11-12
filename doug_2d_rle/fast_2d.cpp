#include <cassert>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <immintrin.h>
#include <iostream>
#include <thread>

#include "concurrentqueue.h"

#define SERIALISER_BUFFER_SIZE 2048

#include "../Input/BasicInput.h"
#include "Serialiser.hpp"

// (x_count + 1) * y_count + 1
#define INPUT_PLANE_SIZE (256 + 1) * 256 + 1

struct Layer {
    char* data;
    uint16_t z;
};

BasicInput* input;

std::atomic_flag finished;
moodycamel::ConcurrentQueue<Layer> input_buffers;
moodycamel::ConcurrentQueue<char*> used_buffers;

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

                if (next_block_mask != block_mask) break;

                chunk_masks[next_mask_index] &= ~block_mask;
                block.y_size += 1;
            }

            serialiser->single(input, block);
        }
        mask_index += (256 / 32);
    }
}

void compression_thread() {
    Serialiser serialiser;

    auto tags = input->get_tags_list();
    BlockSizes block_sizes = input->get_block_sizes();
    block_sizes.chunk_x = 32;
    block_sizes.chunk_y = 32;
    block_sizes.count_x = 256;
    block_sizes.count_y = 256;

    char* input_buffer = nullptr;

    uint64_t tag_masks[(256 / 64) * 256];

    uint16_t z = 0;
    Layer l;
    for (;;) {
        while (!input_buffers.try_dequeue(l)) {
            if (finished.test()) {
                serialiser.flush();
                return;
            }
        }
        std::atomic_thread_fence(std::memory_order_acquire);
        input_buffer = l.data;
        z = l.z;

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
                    compress_chunk((Input*)input, &serialiser, (uint32_t*)tag_masks, tag, chunk_x, chunk_y, z);
                }
            }
        }
        used_buffers.enqueue(input_buffer);
    }
}

int main (int argc, char *argv[]) {
    input = new BasicInput();
    BlockSizes block_sizes = input->get_block_sizes();
    
    uint16_t z = 0;
    char* buf;
    finished.clear();
    std::thread t1(compression_thread);

    for (int i=0; i<4; i++) {
        buf = (char*)malloc(INPUT_PLANE_SIZE);
        std::cin.read(buf, INPUT_PLANE_SIZE);
        Layer layer;
        layer.data = buf;
        layer.z = z++;
        input_buffers.enqueue(layer);
    }

    std::thread t2(compression_thread);
    // std::thread t3(compression_thread);

    for (;z < block_sizes.count_z; z++) {
        while (!used_buffers.try_dequeue(buf));
        std::cin.read(buf, INPUT_PLANE_SIZE);
        Layer layer;
        layer.data = buf;
        layer.z = z;
        std::atomic_thread_fence(std::memory_order_release);
        input_buffers.enqueue(layer);
    }
    finished.test_and_set();

    t1.join();
    t2.join();
    // t3.join();
}
