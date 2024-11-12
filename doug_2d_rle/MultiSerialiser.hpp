#ifndef SERIALISER_H
#define SERIALISER_H

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <thread>
#include "../Input/Input.h"

#ifndef SERIALISER_BUFFER_SIZE
#define SERIALISER_BUFFER_SIZE 1024
#endif

#ifndef SERIALISER_MAX_LINE_SIZE
#define SERIALISER_MAX_LINE_SIZE 50
#endif

extern std::atomic_flag using_stdin;

struct CompressedBlock {
    uint16_t x_pos, y_pos, z_pos;
    uint8_t x_size, y_size, z_size;
    uint8_t tag;
};

class Serialiser {
public:
    static void writing_thread(Serialiser* self) {
        while (self->writing_offset != SIZE_MAX) {
            char *b = self->writing_buffer.load(std::memory_order_acquire);
            if (b == nullptr) continue;
            std::cout.write(b, self->writing_offset);
            self->writing_buffer.store(nullptr, std::memory_order_relaxed);
        }
    }

    Serialiser() {
        writing_buffer.store(nullptr);
        buffer1 = (char*)malloc(SERIALISER_BUFFER_SIZE);
        buffer2 = (char*)malloc(SERIALISER_BUFFER_SIZE);
        active_buffer = buffer1;
        t = new std::thread(writing_thread, this);
    }

    ~Serialiser() {
        writing_offset = SIZE_MAX;
        std::atomic_thread_fence(std::memory_order_release);
        t->join();
    }

    static inline size_t intToBuffer(char *buffer, uint16_t value) {
        char *start = buffer;

        if (value < 10) {
            *buffer++ = '0' + value;
            return 1;
        } else if (value < 100) {
            *buffer++ = '0' + (value / 10);
            *buffer++ = '0' + (value % 10);
            return 2;
        } else if (value < 1000) {
            *buffer++ = '0' + (value / 100);
            value %= 100;
            *buffer++ = '0' + (value / 10);
            *buffer++ = '0' + (value % 10);
            return 3;
        } else if (value < 10000) {
            *buffer++ = '0' + (value / 1000);
            value %= 1000;
            *buffer++ = '0' + (value / 100);
            value %= 100;
            *buffer++ = '0' + (value / 10);
            *buffer++ = '0' + (value % 10);
            return 4;
        } else {
            *buffer++ = '0' + (value / 10000);
            value %= 10000;
            *buffer++ = '0' + (value / 1000);
            value %= 1000;
            *buffer++ = '0' + (value / 100);
            value %= 100;
            *buffer++ = '0' + (value / 10);
            *buffer++ = '0' + (value % 10);
            return 5;
        }
    }

    void single(Input* input, CompressedBlock &block) {
        // If not enough space is available, flush the buffer
        if (offset + SERIALISER_MAX_LINE_SIZE >= SERIALISER_BUFFER_SIZE) {
            flush();
        }

        // Write the block fields into the active_buffer
        offset += intToBuffer(active_buffer + offset, block.x_pos);
        active_buffer[offset++] = ',';
        offset += intToBuffer(active_buffer + offset, block.y_pos);
        active_buffer[offset++] = ',';
        offset += intToBuffer(active_buffer + offset, block.z_pos);
        active_buffer[offset++] = ',';
        offset += intToBuffer(active_buffer + offset, block.x_size);
        active_buffer[offset++] = ',';
        offset += intToBuffer(active_buffer + offset, block.y_size);
        active_buffer[offset++] = ',';
        offset += intToBuffer(active_buffer + offset, block.z_size);
        active_buffer[offset++] = ',';

        // Copy the string tag into the active_buffer
        char* label = input->get_tag_label(block.tag);
        while (*label != 0) {
            active_buffer[offset++] = *label;
            label++;
        }

        active_buffer[offset++] = '\n';
    }

    void serialise(Input* input, CompressedBlock *blocks, uint32_t count) {
        for (int i = 0; i < count; i++) {
            single(input, blocks[i]);
        }
    }

    inline void flush() {
        writing_offset = offset;
        while (writing_buffer.load(std::memory_order_relaxed) != 0);
        writing_buffer.store(active_buffer, std::memory_order_release);
        active_buffer = active_buffer == buffer1 ? buffer2 : buffer1;
        offset = 0;
    }

private:
    std::thread* t;
    size_t offset = 0;
    size_t writing_offset = 0;
    char* active_buffer;
    std::atomic<char*> writing_buffer;
    char* buffer1;
    char* buffer2;
};

#endif
