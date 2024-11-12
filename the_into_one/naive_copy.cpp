#include <cstdint>
#include <cstring>
#include <iostream>

#define SERIALISER_BUFFER_SIZE 19 * 64 * 64 * 64

// this is actually a table of 3 byte null terminated strings
uint32_t label_table[144];
char tag_list[8];

size_t offset = 0;
char buffer[SERIALISER_BUFFER_SIZE];

inline void intToBuffer(char *buffer, uint16_t value) {
    *buffer++ = '0' + (value / 10);
    *buffer++ = '0' + (value % 10);
}

int main (int argc, char *argv[]) {
    // initalise tag table
    for (uint8_t i = 0; i < 144; i++) {
        label_table[i] = 0;
    }

    // ignore first line
    std::string line;
    char tag;
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

    // block template
    char block[19];
    memcpy(block, "00,00,00,1,1,1,   ", 19);

    char* buffer_index = buffer;
    for (uint16_t z=0; z < 64; z++) {
        intToBuffer(block + 6, z);
        for (uint16_t y=0; y < 64; y++) {
            intToBuffer(block + 3, y);
            for (uint16_t x=0; x < 64; x++) {
                intToBuffer(block, x);
                uint8_t tag = *input_chars++;

                *(uint32_t*)(block + 15) = label_table[tag - '!'];

                memcpy(buffer_index, block, 19);

                buffer_index += 19;
            }
            input_chars++;
        }
        input_chars++;
    }

    std::cout.write(buffer, SERIALISER_BUFFER_SIZE);

    return 0;
}
