#include "FirstInput.h"
#include "Input.h"
#include <cstdint>
#include <cstring>
#include <iostream>

FirstInput::FirstInput() {
    /* Buffer to store input data before processing */
    std::string str;

    /* read in x_count, y_count, z_count, parent_x, parent_y, parent_z */
    std::getline(std::cin, str);

    /* Values used to parse the natural numbers by comma separation */
    size_t comma_start = 0;
    size_t comma_end = str.find(',');

    /* parse x_count, y_count, ... */
    /* NOTE: this is hard coded to read in 6 natural numbers, separated
    * specifically by the given input format. Should that format change, this
    * code must also. */

    block_sizes.count_x = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    block_sizes.count_y = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    block_sizes.count_z = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    block_sizes.chunk_x = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    block_sizes.chunk_y = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    block_sizes.chunk_z = std::stoi(str.substr(comma_start));

    char ignore;
    /* read next line */
    std::getline(std::cin, str);

    /* Loop to read in tag table data from standard input until newling is read */
    while (str.length() > 3) {
        /* tag is the first character of the line */
        char tag = str.at(0);
        /* tag and label are separated by ", " so the first character of substr is
        * pos 3 */
        std::string label = str.substr(3, -1);

        tag_table.insert({tag, label});
        tag_list.push_back(tag);

        /* read next line */
        std::getline(std::cin, str);
    }
}

char* FirstInput::get_tag_label(uint8_t tag) {
    return (char*)tag_table.at(tag).c_str();
}

std::vector<uint8_t> FirstInput::get_tags_list() {
    return tag_list;
}

void FirstInput::get_z_layer(uint8_t* chunk) {
    std::string str;
    size_t count = 0;
    /* Loop to read in blocks */
    for (int i = 0; i < block_sizes.chunk_z; i++) {
        for (int j = 0; j < block_sizes.count_y; j++) {
            std::getline(std::cin, str);
            /* skip blank lines */
            if (str.compare("") == 0) continue;

            std::memcpy(chunk + count, str.c_str(), str.length());
            count += str.length();
        }
        /* skip blank line separating slices */
        std::getline(std::cin, str);
    }
}
