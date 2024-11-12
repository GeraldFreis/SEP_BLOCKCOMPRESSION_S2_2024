#include "../Input/BasicInput.h"
#include <cstdio>
#include <cstring>

int main (int argc, char *argv[]) {
    BasicInput input;
    auto block_sizes = input.get_block_sizes();

    auto tags = input.get_tags_list();
    int longest = 0;
    int shortest = 999;
    for (auto tag : tags) {
        char* label = input.get_tag_label(tag);
        int length = strlen(label);
        if (length > longest) {
            longest = length;
        }
        if (length < shortest) {
            shortest = length;
        }
    }

    printf("count x %lu count y %lu count z %lu chunk x %lu chunk y %lu chunk z %lu tag shortest %d longest %d count %d\n",
           block_sizes.count_x,
           block_sizes.count_y,
           block_sizes.count_z,
           block_sizes.chunk_x,
           block_sizes.chunk_y,
           block_sizes.chunk_z,
           shortest,
           longest,
           tags.size());
    
    return 0;
}
