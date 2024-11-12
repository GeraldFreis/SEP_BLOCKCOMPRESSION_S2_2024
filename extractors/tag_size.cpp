#include "../Input/BasicInput.h"
#include <cstring>

int main (int argc, char *argv[]) {
    BasicInput input;
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
    
    return (shortest << 16) | (longest << 8) | tags.size();
}
