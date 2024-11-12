#ifndef BASICINPUT_H
#define BASICINPUT_H

#include "Input.h"
#include <cstdint>

class BasicInput : public Input {
public:
    BasicInput();

    char* get_tag_label(uint8_t tag);
    std::vector<uint8_t> get_tags_list();
    void get_z_layer(uint8_t* chunk);
    void get_x_line(uint8_t* chunk);
private:
    // holds offsets into the label_strings vector
    // allowed ranges 33..=176, base renderable ascii, starts at '!'
    uint16_t tag_table[144];
    // internalled labels, a vector is used here instead of a string
    // to allow pushing back after null terminators
    // total accumilated strings can not go above 65,535
    std::vector<char> labels_string;
    // a list of tags for iterating through
    std::vector<uint8_t> tag_list;
};

#endif
