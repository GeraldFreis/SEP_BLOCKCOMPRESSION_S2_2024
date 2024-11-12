#ifndef FIRSTINPUT_H
#define FIRSTINPUT_H

#include "Input.h"
#include <cstdint>
#include <map>

class FirstInput : public Input {
public:
    FirstInput();

    char* get_tag_label(uint8_t tag);
    std::vector<uint8_t> get_tags_list();
    void get_z_layer(uint8_t* chunk);
private:
    /* map to store tag-label pairs */
    std::map<uint8_t, std::string> tag_table;
    std::vector<uint8_t> tag_list;
};

#endif
