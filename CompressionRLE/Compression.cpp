#include "Compression.h"
#include <vector>
#include <string>
#include <map>
#include <iostream>

Compression::Compression(std::vector<std::vector<std::string>> blocks_array, int *headers_array, std::map<char, std::string> table){
    blocks = blocks_array;
    headers = headers_array;
    tag_table = table;
}

std::string getLabel(char tag, std::map<char, std::string> table )  { 
    auto it = table.find(tag);
    if (it != table.end()) return it->second;
    else return "unknown"; 
}


void Compression::runLengthEncoding(){
    int x_count, y_count, z_count, parent_x, parent_y, parent_z;
    x_count = headers[0]; y_count = headers[1]; z_count=headers[2]; parent_x = headers[3]; parent_y = headers[4]; parent_z = headers[5];
    for (int z = 0; z < z_count; z++) {
        for (int y = 0; y < y_count; y++) {
            for (int x = 0; x < x_count; x++) {

                char label = blocks[z][y][x];

                // Determine parent block boundaries
                int parent_x_start = (x / parent_x) * parent_x;
                int parent_y_start = (y / parent_y) * parent_y;
                int parent_z_start = (z / parent_z) * parent_z;

                int parent_x_end = parent_x_start + parent_x;
                int parent_y_end = parent_y_start + parent_y;
                int parent_z_end = parent_z_start + parent_z;

                // Determine maximum block size within the parent block
                int max_x_size = 1;
                while (x + max_x_size < parent_x_end &&
                       blocks[z][y][x + max_x_size] == label && x+max_x_size < x_count) {
                    max_x_size++;
                }

                int max_y_size = 1;
                // while (y + max_y_size < parent_y_end && y + max_y_size < y_count) {
                //     bool uniform_y = true;
                //     for (int dx = 0; dx < max_x_size; dx++) {
                //         if (block[z][y + max_y_size][x + dx] != label) {
                //             uniform_y = false;
                //             break;
                //         }
                //     }
                //     if (uniform_y) {
                //         max_y_size++;
                //     } else {
                //         break;
                //     }
                // }

                int max_z_size = 1;
                // (If applicable) You can apply similar logic to the z dimension.

                // Output the compressed block within parent boundaries
                std::string label_str = getLabel(label, tag_table);
                std::string output_line = std::to_string(x) + "," +
                                          std::to_string(y) + "," +
                                          std::to_string(z) + "," +
                                          std::to_string(max_x_size) + "," +
                                          std::to_string(max_y_size) + "," +
                                          std::to_string(max_z_size) + "," +
                                          label_str;

                std::cout << output_line << "\n";

                // Move to the next block after the compressed block
                x += max_x_size - 1;  // Subtract 1 because the loop will increment x
            }
        }
    }

}