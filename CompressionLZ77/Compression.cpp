#include "./Compression.h"
std::string getLabel(char tag, std::map<char, std::string> table )  { 
    auto it = table.find(tag);
    if (it != table.end()) return it->second;
    else return "unknown"; 
}
Compression::Compression(std::vector<std::vector<std::string>> blocks_array, int* headers_array, std::map<char, std::string> table) {
  blocks = blocks_array;
  headers = headers_array;
  tag_table = table;
  // Initialize visited array with false
  visited.resize(headers[2], std::vector<std::vector<bool>>(headers[1], std::vector<bool>(headers[0], false)));
}

bool Compression::withinBounds(int x, int y, int z, int px, int py, int pz) {
            return x < px && y < py && z < pz;
        }


void Compression::compressBlock(int start_x, int start_y, int start_z) {
    // Define boundaries for the block, constrained by parent block sizes
    int max_x = std::min(start_x + headers[3], headers[0]);  // Maximum width allowed
    int max_y = std::min(start_y + headers[4], headers[1]);  // Maximum height allowed
    int max_z = std::min(start_z + headers[5], headers[2]);  // Maximum depth allowed


    // Use a simple strategy to group identical blocks into larger sub-blocks
    for (int z = start_z; z <= max_z; ++z) {  // Change to <= for inclusive max_z
        for (int y = start_y; y <= max_y; ++y) {  // Change to <= for inclusive max_y
            for (int x = start_x; x <= max_x; ++x) {  // Change to <= for inclusive max_x
                
                char label = blocks[z][y][x];
                if (label == '*' || label == NULL) { continue; }
                else {
                    int x_len = 1, y_len = 1, z_len = 1;

                    // Attempt to expand the block in x, y, and z directions where possible
                    while (x + x_len < max_x && blocks[z][y][x + x_len] == label) {
                        ++x_len;
                    }
                    while (y + y_len < max_y && checkRowEqual(z, y + y_len, x, x_len, label)) {
                        ++y_len;
                    }
                    while (z + z_len < max_z && checkSliceEqual(z + z_len, y, x, x_len, y_len, label)) {
                        ++z_len;
                    }

                    // Output the compressed block
                    std::cout << x << "," << y << "," << z << "," << x_len << "," << y_len << "," << z_len << "," << getLabel(label, tag_table) << std::endl;

                    // Mark the blocks as processed by setting them to a special value (e.g., '*')
                    markBlockProcessed(x, y, z, x_len, y_len, z_len);
                }
            }
        }
    }
}


    // Check if a row can be merged (all blocks are equal to the current label)
    bool Compression::checkRowEqual(int z, int y, int x, int x_len, char label) {
        for (int i = 0; i < x_len; ++i) {
            if (blocks[z][y][x + i] != label) {
                return false;
            }
        }
        return true;
    }

    // Check if a slice can be merged (all blocks in the slice are equal to the current label)
    bool Compression::checkSliceEqual(int z, int y, int x, int x_len, int y_len, char label) {
        for (int j = 0; j < y_len; ++j) {
            for (int i = 0; i < x_len; ++i) {
                if (blocks[z][y + j][x + i] != label) {
                    return false;
                }
            }
        }
        return true;
    }

    // Mark blocks as processed to avoid duplicate processing
    void Compression::markBlockProcessed(int x, int y, int z, int x_len, int y_len, int z_len) {
        for (int k = 0; k < z_len; ++k) {
            for (int j = 0; j < y_len; ++j) {
                for (int i = 0; i < x_len; ++i) {
                    blocks[z + k][y + j][x + i] = '*';  // Mark as processed
                }
            }
        }
    }



void Compression::compress() {
  
    for (int z = 0; z < headers[2]; z += headers[5]) {
        for (int y = 0; y < headers[1]; y += headers[4]) {
            for (int x = 0; x < headers[0]; x += headers[3]) {
                compressBlock(x, y, z);
            }
        }
    }    
}