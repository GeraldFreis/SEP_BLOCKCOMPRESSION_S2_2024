#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>

#include "../../Input/FirstInput.h"
#include "../../Input/BasicInput.h"

#define RANDOMCOUNT 300000000

int main (int argc, char *argv[]) {
    if (argc < 2) {
        printf("specify which input to use [f/b]\n");
        return 1;
    }
    Input* input = nullptr;
    switch (argv[1][0]) {
        case 'f':
            input = new FirstInput;
        break;
        case 'b':
            input = new BasicInput;
        break;
        default: {
            printf("invalid\n");
            return -1;
        }
    }
    // setup
    auto tags_list = input->get_tags_list();

    size_t count = 0;
    for (size_t i=0; i < (size_t)RANDOMCOUNT; i++) {
        uint16_t tag_index = rand() % tags_list.size();
        uint8_t tag = tags_list[tag_index];
        char* label = input->get_tag_label(tag);

        // sum each
        while (*label != '\0') {
            count += *label++;
        }
    }
    printf("%zu\n", count);
    
    return 0;
}
