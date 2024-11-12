#ifndef SERIALISER_H
#define SERIALISER_H

#include <cstddef>
#include <iostream>
#include "../Input/Input.h"

#ifndef SERIALISER_BUFFER_SIZE
#define SERIALISER_BUFFER_SIZE 1024
#endif

#ifndef SERIALISER_MAX_LINE_SIZE
#define SERIALISER_MAX_LINE_SIZE 50
#endif

struct CompressedBlock {
    uint16_t x_pos, y_pos, z_pos;
    uint8_t x_size, y_size, z_size;
    uint8_t tag;
};

class Serialiser {
public:

    Serialiser() {
#ifndef GLOBAL
        buffer = (char*)malloc(SERIALISER_BUFFER_SIZE);
#endif
    }

    ~Serialiser() {
#ifndef GLOBAL
        free(buffer);
#endif
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
        }
        return 0;
    }

    void single(Input* input, CompressedBlock &block) {
        // If not enough space is available, flush the buffer
        if (offset + SERIALISER_MAX_LINE_SIZE >= SERIALISER_BUFFER_SIZE) {
            flush();
        }

        // Write the block fields into the buffer
        offset += intToBuffer(buffer + offset, block.x_pos);
        buffer[offset++] = ',';
        offset += intToBuffer(buffer + offset, block.y_pos);
        buffer[offset++] = ',';
        offset += intToBuffer(buffer + offset, block.z_pos);
        buffer[offset++] = ',';
        offset += intToBuffer(buffer + offset, block.x_size);
        buffer[offset++] = ',';
        offset += intToBuffer(buffer + offset, block.y_size);
        buffer[offset++] = ',';
        offset += intToBuffer(buffer + offset, block.z_size);
        buffer[offset++] = ',';

        // Copy the string tag into the buffer
        char* label = input->get_tag_label(block.tag);
        while (*label != 0) {
            buffer[offset++] = *label;
            label++;
        }

        buffer[offset++] = '\n';
    }

    void serialise(Input* input, CompressedBlock *blocks, uint32_t count) {
        for (int i = 0; i < count; i++) {
            single(input, blocks[i]);
        }
    }

    inline void flush() {
        std::cout.write(buffer, offset);
        offset = 0;
    }

private:
    size_t offset = 0;

    #ifdef GLOBAL
    char buffer[SERIALISER_BUFFER_SIZE];
    #else
    char* buffer;
    #endif
};

#endif
