#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>
#include <unordered_map>
#include <tuple>

/* Define Compressor Guard */
#ifndef Compressor

class Input {
private:
    int x_count, y_count, z_count, parent_x, parent_y, parent_z;
    std::map<char, std::string> tag_table;
    std::vector<std::vector<std::string>> current_slice;

public:
    Input();
    void readIn();
    void blocksInit();
    void printBlocks();
    void printTable();
    std::vector<std::vector<std::string>> getBlocks();
    std::vector<std::vector<std::string>> getSlice(int z_start);
    int getParentX() { return parent_x; }
    int getParentY() { return parent_y; }
    int getParentZ() { return parent_z; }
    char getLabel(int x, int y, int z);
    std::string getLabel(char tag) const;
};

/* Default Constructor */
Input::Input() {}

void Input::readIn() {
    std::string str;
    std::getline(std::cin, str);

    size_t comma_start = 0;
    size_t comma_end = str.find(',');

    x_count = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    y_count = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    z_count = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    parent_x = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    parent_y = std::stoi(str.substr(comma_start, comma_end));
    comma_start = comma_end + 1;
    comma_end = str.find(',', comma_start);

    parent_z = std::stoi(str.substr(comma_start));

    // Read tag table
    while (std::getline(std::cin, str)) {
        if (str.empty()) break; // Blank line indicates end of tag table
        if (str.length() < 3) continue; // Invalid tag entry
        char tag = str.at(0);
        std::string label = str.substr(2); // Changed from 3 to 2 to correctly parse after comma
        tag_table.insert({ tag, label });
    }

    blocksInit();
}

void Input::blocksInit() {
    if (x_count <= 0 || y_count <= 0 || z_count <= 0) {
        std::cerr << "Dimensions are non-natural: x: " << x_count
                  << ", y: " << y_count << ", z: " << z_count << std::endl;
        exit(1);
    }

    std::string line(x_count, ' ');
    current_slice.resize(parent_z, std::vector<std::string>(y_count, line));
}

std::vector<std::vector<std::string>> Input::getSlice(int z_start) {
    std::string str;

    int z_end = std::min(z_start + parent_z, z_count);
    for (int z = z_start; z < z_end; ++z) {
        for (int y = 0; y < y_count; ++y) {
            std::getline(std::cin, str);
            if (str.empty()) {
                // Handle unexpected blank lines within block data
                y--;
                continue;
            }
            if (str.size() != static_cast<size_t>(x_count)) {
                std::cerr << "Invalid block line length at z=" << z << ", y=" << y << std::endl;
                exit(1);
            }
            current_slice[z - z_start][y] = str;
        }
        // Read the blank line separating slices
        std::getline(std::cin, str);
    }
    return current_slice;
}

char Input::getLabel(int x, int y, int z) {
    return current_slice[z % parent_z][y][x];
}

std::string Input::getLabel(char tag) const { 
    auto it = tag_table.find(tag);
    if (it != tag_table.end()) return it->second;
    else return "unknown"; 
}



void Input::printTable() {
    for (const auto& pair : tag_table) {
        std::cout << "tag: " << pair.first << ", label: " << pair.second << std::endl;
    }
}


// Helper function to convert block parameters to a tuple for easier management
std::tuple<int, int, int, int, int, int, std::string> parseBlock(const std::string& block) {
    int x_start, y_start, z_start, x_size, y_size, z_size;
    char label_char;
    // Temporarily store the label as a char
    sscanf(block.c_str(), "%d,%d,%d,%d,%d,%d,%c", &x_start, &y_start, &z_start, &x_size, &y_size, &z_size, &label_char);
    std::string label(1, label_char); // Convert to string
    return std::make_tuple(x_start, y_start, z_start, x_size, y_size, z_size, label);
}


class Compressor {
private:
    Input input;
    std::vector<std::string> output;
    std::vector<std::vector<std::string>> blocks_array;
    int parent_x, parent_y, parent_z;

    // Modify isUniform to use string labels
    bool isUniform(int x_start, int y_start, int z_start, int x_size, int y_size, int z_size) {
        char first_label = blocks_array[z_start][y_start][x_start];
        for (int z = z_start; z < z_start + z_size; ++z) {
            for (int y = y_start; y < y_start + y_size; ++y) {
                for (int x = x_start; x < x_start + x_size; ++x) {
                    if (blocks_array[z][y][x] != first_label) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    // Compress a uniform block and add to the temporary output vector
    void compressBlock(int x_start, int y_start, int z_start, int x_size, int y_size, int z_size, char label, std::vector<std::string>& temp_output) {
        std::string line = std::to_string(x_start) + "," + std::to_string(y_start) + "," + std::to_string(z_start) + ",";
        line += std::to_string(x_size) + "," + std::to_string(y_size) + "," + std::to_string(z_size) + "," + input.getBlocks()[z_start][y_start][x_start];
        temp_output.push_back(line);
    }

    // Combine blocks within a parent block without crossing boundaries
    void combineBlocks(std::vector<std::string>& temp_output, int parent_x_start, int parent_y_start, int parent_z_start) {
        std::unordered_map<std::string, bool> block_map;
        std::vector<std::string> combined_output;

        // Populate the block_map with all blocks in temp_output
        for (const auto& block : temp_output) {
            block_map[block] = false;
        }

        // Iterate over the blocks and try to combine adjacent blocks
        for (const auto& block_entry : block_map) {
            if (block_entry.second) continue; // Skip already combined blocks

            std::string current = block_entry.first;
            auto [x_start_1, y_start_1, z_start_1, x_size_1, y_size_1, z_size_1, label_1] = parseBlock(current);

            bool has_merged = false;

            for (const auto& other_entry : block_map) {
                if (current == other_entry.first || other_entry.second) continue;

                std::string next = other_entry.first;
                auto [x_start_2, y_start_2, z_start_2, x_size_2, y_size_2, z_size_2, label_2] = parseBlock(next);

                // Check if blocks are adjacent and can be merged without crossing parent boundaries
                if (label_1 == label_2) {
                    // Check x direction
                    if (y_start_1 == y_start_2 && z_start_1 == z_start_2 &&
                        y_size_1 == y_size_2 && z_size_1 == z_size_2 &&
                        x_start_1 + x_size_1 == x_start_2 &&
                        (x_size_1 + x_size_2) <= parent_x) {
                        // Merge in x direction
                        current = std::to_string(x_start_1) + "," + std::to_string(y_start_1) + "," + std::to_string(z_start_1) + ",";
                        current += std::to_string(x_size_1 + x_size_2) + "," + std::to_string(y_size_1) + "," + std::to_string(z_size_1) + "," + label_1;
                        has_merged = true;
                        block_map[next] = true; // Mark as combined
                    }
                    // Check y direction
                    else if (x_start_1 == x_start_2 && z_start_1 == z_start_2 &&
                             x_size_1 == x_size_2 && z_size_1 == z_size_2 &&
                             y_start_1 + y_size_1 == y_start_2 &&
                             (y_size_1 + y_size_2) <= parent_y) {
                        // Merge in y direction
                        current = std::to_string(x_start_1) + "," + std::to_string(y_start_1) + "," + std::to_string(z_start_1) + ",";
                        current += std::to_string(x_size_1) + "," + std::to_string(y_size_1 + y_size_2) + "," + std::to_string(z_size_1) + "," + label_1;
                        has_merged = true;
                        block_map[next] = true; // Mark as combined
                    }
                    // Check z direction
                    else if (x_start_1 == x_start_2 && y_start_1 == y_start_2 &&
                             x_size_1 == x_size_2 && y_size_1 == y_size_2 &&
                             z_start_1 + z_size_1 == z_start_2 &&
                             (z_size_1 + z_size_2) <= parent_z) {
                        // Merge in z direction
                        current = std::to_string(x_start_1) + "," + std::to_string(y_start_1) + "," + std::to_string(z_start_1) + ",";
                        current += std::to_string(x_size_1) + "," + std::to_string(y_size_1) + "," + std::to_string(z_size_1 + z_size_2) + "," + label_1;
                        has_merged = true;
                        block_map[next] = true; // Mark as combined
                    }

                    if (has_merged) {
                        break; // After merging, restart to check for further possible merges
                    }
                }
            }

            combined_output.push_back(current);
        }

        // Replace temp_output with the combined_output
        temp_output = combined_output;
    }

    // Process a single parent block and collect compressed blocks
    void processParentBlock(int x_start, int y_start, int z_start, int x_size, int y_size, int z_size, std::vector<std::string>& temp_output) {
        if (x_size <= 0 || y_size <= 0 || z_size <= 0) return;
        if(x_start > x_size || y_start > y_size || z_start > z_size) return;
        if (isUniform(x_start, y_start, z_start, x_size, y_size, z_size)) {
            char label = blocks_array[z_start][y_start][x_start];
            compressBlock(x_start, y_start, z_start, x_size, y_size, z_size, label, temp_output);
        } else {
            // Determine the next subdivision size (you can choose different strategies here)
            int new_x_size = x_size / 2;
            int new_y_size = y_size / 2;
            int new_z_size = z_size / 2;

            // Ensure that the subdivision size is at least 1
            new_x_size = std::max(new_x_size, 1);
            new_y_size = std::max(new_y_size, 1);
            new_z_size = std::max(new_z_size, 1);

            // Recursively subdivide the block into 8 octants
            processParentBlock(x_start, y_start, z_start, new_x_size, new_y_size, new_z_size, temp_output);
            processParentBlock(x_start + new_x_size, y_start, z_start, x_size - new_x_size, new_y_size, new_z_size, temp_output);
            processParentBlock(x_start, y_start + new_y_size, z_start, new_x_size, y_size - new_y_size, new_z_size, temp_output);
            processParentBlock(x_start + new_x_size, y_start + new_y_size, z_start, x_size - new_x_size, y_size - new_y_size, new_z_size, temp_output);

            processParentBlock(x_start, y_start, z_start + new_z_size, new_x_size, new_y_size, z_size - new_z_size, temp_output);
            processParentBlock(x_start + new_x_size, y_start, z_start + new_z_size, x_size - new_x_size, new_y_size, z_size - new_z_size, temp_output);
            processParentBlock(x_start, y_start + new_y_size, z_start + new_z_size, new_x_size, y_size - new_y_size, z_size - new_z_size, temp_output);
            processParentBlock(x_start + new_x_size, y_start + new_y_size, z_start + new_z_size, x_size - new_x_size, y_size - new_y_size, z_size - new_z_size, temp_output);
        }
    }

public:
    void printOutput() {
        for (const auto& line : output) {
            std::cout << line << std::endl;
        }
    }

    void compress() {
        input.readIn();

        parent_x = input.getParentX();
        parent_y = input.getParentY();
        parent_z = input.getParentZ();

        int z_count = input.getBlocks().size();

        // Iterate over each parent block in slices
        for (int z = 0; z < z_count; z += parent_z) {
            // Load the next slice of blocks
            auto current_slice = input.getSlice(z);
            int x_count = current_slice[0].size();
            int y_count = current_slice.size();

            // Process and compress blocks within the current slice
            for (int y = 0; y < y_count; y += parent_y) {
                for (int x = 0; x < x_count; x += parent_x) {
                    int current_x_size = std::min(parent_x, x_count - x);
                    int current_y_size = std::min(parent_y, y_count - y);
                    int current_z_size = std::min(parent_z, z_count - z);

                    // Temporary output for the current parent block
                    std::vector<std::string> temp_output;

                    // Process and compress the current parent block
                    processParentBlock(x, y, 0, current_x_size, current_y_size, current_z_size, temp_output);

                    // Merge blocks within the current parent block
                    combineBlocks(temp_output, x, y, z);

                    // Add the merged blocks to the global output
                    for (const auto& block : temp_output) {
                        // Convert label character to its corresponding string label
                        auto parsed_block = parseBlock(block);
                        int x_pos = std::get<0>(parsed_block);
                        int y_pos = std::get<1>(parsed_block);
                        int z_pos = std::get<2>(parsed_block);
                        int x_size_out = std::get<3>(parsed_block);
                        int y_size_out = std::get<4>(parsed_block);
                        int z_size_out = std::get<5>(parsed_block);

                        char label_char = input.getLabel(x_pos, y_pos, z_pos);
                        std::string label_str = input.getLabel(label_char);

                        // Format the output line as specified
                        std::string output_line = std::to_string(x_pos) + "," + std::to_string(y_pos) + "," + std::to_string(z_pos + z) + ",";
                        output_line += std::to_string(x_size_out) + "," + std::to_string(y_size_out) + "," + std::to_string(z_size_out) + "," + label_str;

                        output.push_back(output_line);
                    }
                }
            }
        }

        // No global merging after all parent blocks are processed
    }
};

/* Add the getLabel method to the Input class */
std::string getLabelFromTag(const Input& input, char tag) {
    return input.getLabel(tag);
}

#endif

int main() {
    Compressor compressor;
    compressor.compress();
    compressor.printOutput();
    return 0;
}