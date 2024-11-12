#include <cstdint>
#include <cstring>
#include <iostream>
#include <immintrin.h>

#define SERIALISER_BUFFER_SIZE 19 * 64 * 64 * 32

// this is actually a table of 3 byte null terminated strings
uint32_t label_table[144];
char tag_list[8];

char buffer[SERIALISER_BUFFER_SIZE];

inline uintptr_t intToBuffer(char *buffer, uint16_t value) {
    *buffer++ = '0' + (value / 10);
    *buffer++ = '0' + (value % 10);
    return 2;
}

int main (int argc, char *argv[]) {
    // initalise tag table
    memset(label_table, 0, 144);

    char tag;
    // ignore first line
    std::string line;
    std::getline(std::cin, line);

    // read in tags
    for (int i=0; i < 8; i++) {
        std::getline(std::cin, line);

        uint8_t tag = line[0];
        tag_list[i] = tag;

        label_table[tag - '!'] = (uint32_t)0x0a202020; // "   \n"
        
        char* index = (char*)line.c_str() + line.length();

        char c = *(--index);
        int offset = 2;
        while (c != ' ' && c != ',') {
            ((char*)&label_table[tag - '!'])[offset--] = c;
            c = *(--index);
        }
    }

    // skip empty line
    std::getline(std::cin, line);

    // reading in data
    char* input_chars = (char*)malloc(65 * 65 * 64);
    std::cin.read(input_chars, 65 * 65 * 64);

    char* buffer_index = buffer;
    for (uint16_t z=0; z < 64; z++) {
        for (uint16_t y=0; y < 64; y++) {

            // load input into simd registers
            __m512i line = _mm512_loadu_epi8((const __m512i*)input_chars);

            for (uint8_t i=0; i < 8; i++) {
                uint8_t tag = tag_list[i];
                uint32_t label = label_table[tag - '!'];

                // spread tag out
                __m512i tag_mask = _mm512_set1_epi8(tag);

                // compare to bitmask
                uint64_t cmp_mask = _mm512_cmpeq_epi8_mask(line, tag_mask);

                uint16_t x = 0;
                while (cmp_mask != 0) {
                    // find start of block
                    uint16_t leading_zero = std::__countr_zero(cmp_mask);
                    cmp_mask >>= leading_zero;
                    x += leading_zero;

                    // get length
                    uint64_t parent_mask = 0xf >> (x % 4);
                    uint16_t length = std::__countr_one(cmp_mask & parent_mask);

                    buffer_index += intToBuffer(buffer_index, x);
                    *buffer_index++ = ',';
                    buffer_index += intToBuffer(buffer_index, y);
                    *buffer_index++ = ',';
                    buffer_index += intToBuffer(buffer_index, z);
                    *buffer_index++ = ',';
                    buffer_index += intToBuffer(buffer_index, length);
                    *buffer_index++ = ',';
                    *buffer_index++ = '1';
                    *buffer_index++ = ',';
                    *buffer_index++ = '1';
                    *buffer_index++ = ',';
                    *(uint32_t*)(buffer_index) = label;
                    buffer_index += 4;

                    // shift off block
                    cmp_mask >>= length;
                    x += length;
                }
            }

            input_chars += 65;
        }
        input_chars++;
    }

    std::cout.write(buffer, buffer_index - buffer);

    return 0;
}
