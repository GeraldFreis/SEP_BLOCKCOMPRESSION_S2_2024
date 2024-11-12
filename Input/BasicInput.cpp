#include "BasicInput.h"
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <iostream>

BasicInput::BasicInput() {
    // initalise tag table
    for (uint8_t i = 0; i < 144; i++) {
        tag_table[i] = 0;
    }

    // read in x_count, y_count, z_count, parent_x, parent_y, parent_z
    int _ = scanf("%zu,%zu,%zu,%zu,%zu,%zu", 
          &block_sizes.count_x,
          &block_sizes.count_y,
          &block_sizes.count_z,
          &block_sizes.chunk_x,
          &block_sizes.chunk_y,
          &block_sizes.chunk_z);

    char tag;
    char label[31];
    std::string line;

    // clear the remaining character on the first line
    std::getline(std::cin, line);
    // push single char so first string is at index > 0
    labels_string.push_back('\0');
    tag_list.reserve(20);

    // read in tags
    while (true) {
        std::getline(std::cin, line);
        if (line.empty()) break;

        sscanf(line.c_str(), "%c,%30s", &tag, label);

        tag_list.push_back(tag);
        tag_table[tag - '!'] = (uint16_t)labels_string.size();
        for (char c : label) {
            labels_string.push_back(c);
        }
        labels_string.push_back('\0');
    }
}

inline char* BasicInput::get_tag_label(uint8_t tag) {
    return labels_string.data() + tag_table[tag - '!'];
}

inline std::vector<uint8_t> BasicInput::get_tags_list() {
    return tag_list;
}

inline void BasicInput::get_z_layer(uint8_t* chunk) {
    char ignore;
    size_t count = 0;
    for (int i = 0; i < block_sizes.chunk_z; i++) {
        for (int j = 0; j < block_sizes.count_y; j++) {
            std::cin.read((char*)chunk + count, block_sizes.count_x);
            count += block_sizes.count_x;
            std::cin.read(&ignore, 1);
        }
        std::cin.read(&ignore, 1);
    }
}

inline void BasicInput::get_x_line(uint8_t* chunk) {
    char ignore;
    std::cin.read((char*)chunk, block_sizes.count_x);
    std::cin.read(&ignore, 1);
}
