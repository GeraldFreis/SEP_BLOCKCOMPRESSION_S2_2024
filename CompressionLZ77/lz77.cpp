#include <iostream>
#include <map>
#include <string>
#include <vector>

/// @brief this file contains the lz77 compression model using a sliding window approach
class Input {
 private:
  /*
   * From Block Compression Project Outline:
   * The first three (x_count, y_count, z_count) specify the number of columns,
   * rows and slices in the block model respectively, much like the width and
   * height in pixels of an image but with an additional Z dimension for the
   * number of slices there are or depth. The second three (parent_x, parent_y,
   * parent_z) specify the parent block size in each of these dimensions to use
   * for compression. */
  int x_count, y_count, z_count, parent_x, parent_y, parent_z;
  /* map to store tag-label pairs */
  std::map<char, std::string> tag_table;
  /* 3D vector to store blocks.
   * Each block is represented by a single character,
   * Each row is represented by a string,
   * Each slice is represented by a vector of strings, */
  std::vector<std::vector<std::string>> blocks;

 public:
  Input();
  void readIn();
  void blocksInit();
  void printBlocks();
  void printTable();
  int *getHeaders();
  std::vector<std::vector<std::string>> getBlocks();
  std::map<char, std::string> Input::getTagTable();
};

/* Default Constructor */
Input::Input() {}

void Input::readIn() {
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

  /* read next line */
  std::getline(std::cin, str);

  /* Loop to read in tag table data from standard input until newling is read */
  while (str.length() > 3) {
    /* tag is the first character of the line */
    char tag = str.at(0);
    /* tag and label are separated by ", " so the first character of substr is
     * pos 3 */
    std::string label = str.substr(3);

    tag_table.insert({tag, label});

    /* read next line */
    std::getline(std::cin, str);
  }

  /* initialise blocks vector */
  blocksInit();

  /* Loop to read in blocks */
  for (int i = 0; i < z_count; i++) {
    for (int j = 0; j < y_count; j++) {
      std::getline(std::cin, str);
      /* skip blank lines */
      if (str.compare("") == 0) continue;

      blocks.at(i).at(j) = str;
    }
    /* skip blank line separating slices */
    std::getline(std::cin, str);
  }

  return;
}

/* initialises blocks vector using the private variables x_count, y_count,
 * z_count */
void Input::blocksInit() {
  if (x_count <= 0 || y_count <= 0 || z_count <= 0) {
    std::cerr << "dimensions are non-natural: x: " << x_count
              << ", y: " << y_count << ", z: " << z_count << std::endl;
    return;
  }

  std::string line(x_count, ' ');
  std::vector<std::string> slice(y_count, line);
  blocks.resize(z_count, slice);

  return;
}

void Input::printBlocks() {
  for (int i = 0; i < z_count; i++) {
    for (int j = 0; j < y_count; j++) {
      std::cout << blocks.at(i).at(j) << std::endl;
    }
    std::cout << std::endl;
  }
}

void Input::printTable() {
  std::map<char, std::string>::iterator it;
  for (it = tag_table.begin(); it != tag_table.end(); it++) {
    std::cout << "tag: " << it->first << ", label: " << it->second << std::endl;
  }
}

std::vector<std::vector<std::string>> Input::getBlocks()
{
  return blocks;
}
int *Input::getHeaders(){
    // returning the headers as per specifications
    int *headers = new int[6];
    headers[0] = x_count;
    headers[1] = y_count;
    headers[2] = z_count;
    headers[3] = parent_x;
    headers[4] = parent_y;
    headers[5] = parent_z;
    return headers;
}

std::map<char, std::string> Input::getTagTable(){
    // getting the tag table
    return tag_table;
}


std::string getLabel(char tag, std::map<char, std::string> table )  { 
    auto it = table.find(tag);
    if (it != table.end()) return it->second;
    else return "unknown"; 
}

class Compression {
    // class to handle compressing the input
    private:
        std::vector<std::vector<std::string>> blocks;
        int *headers;
        std::map<char, std::string> tag_table;
        std::vector<std::vector<std::vector<bool>>> visited;

        // Helper function to ensure cursor doesn't exceed boundaries
        bool withinBounds(int x, int y, int z, int px, int py, int pz) {
            return x < px && y < py && z < pz;
        }

    void compressBlock(int start_x, int start_y, int start_z) {
        // Define boundaries for the block, constrained by parent block sizes
        int max_x = std::min(start_x + headers[3], headers[0]);
        int max_y = std::min(start_y + headers[4], headers[1]);
        int max_z = std::min(start_z + headers[5], headers[2]);

        // Use a simple strategy to group identical blocks into larger sub-blocks
        for (int z = start_z; z < max_z; ++z) {
            for (int y = start_y; y < max_y; ++y) {
                for (int x = start_x; x < max_x; ++x) {

                    char label = blocks[z][y][x];
                    if(label == '*') continue;
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

    // Check if a row can be merged (all blocks are equal to the current label)
    bool checkRowEqual(int z, int y, int x, int x_len, char label) {
        for (int i = 0; i < x_len; ++i) {
            if (blocks[z][y][x + i] != label) {
                return false;
            }
        }
        return true;
    }

    // Check if a slice can be merged (all blocks in the slice are equal to the current label)
    bool checkSliceEqual(int z, int y, int x, int x_len, int y_len, char label) {
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
    void markBlockProcessed(int x, int y, int z, int x_len, int y_len, int z_len) {
        for (int k = 0; k < z_len; ++k) {
            for (int j = 0; j < y_len; ++j) {
                for (int i = 0; i < x_len; ++i) {
                    blocks[z + k][y + j][x + i] = '*';  // Mark as processed
                }
            }
        }
    }


    public:
        Compression(std::vector<std::vector<std::string>> blocks, int *headers, std::map<char, std::string> table);
        void compress();
};

Compression::Compression(std::vector<std::vector<std::string>> blocks_array, int* headers_array, std::map<char, std::string> table) {
  blocks = blocks_array;
  headers = headers_array;
  tag_table = table;
  // Initialize visited array with false
  visited.resize(headers[2], std::vector<std::vector<bool>>(headers[1], std::vector<bool>(headers[0], false)));
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



// main function to run this file
int main () {
    // first we need to read in the data
    Input inp;
    inp.readIn();

    Compression lz77(inp.getBlocks(), inp.getHeaders(), inp.getTagTable());
    lz77.compress();
} 