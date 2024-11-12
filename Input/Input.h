#ifndef INPUT_H
#define INPUT_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

struct BlockSizes {
    size_t count_x;
    size_t count_y;
    size_t count_z;
    size_t chunk_x;
    size_t chunk_y;
    size_t chunk_z;
};

class Input {
public:
    virtual char* get_tag_label(uint8_t tag) = 0;
    virtual std::vector<uint8_t> get_tags_list() = 0;
    virtual void get_z_layer(uint8_t* chunk) = 0;

    inline BlockSizes get_block_sizes() {
        return block_sizes;
    }

    inline static size_t vec_to_index(BlockSizes &block_sizes, uint16_t x, uint16_t y, uint16_t z) {
        return (size_t)z * block_sizes.count_y + (size_t)y * block_sizes.count_x + (size_t)x;
    }

protected:
    BlockSizes block_sizes;
};

#endif
