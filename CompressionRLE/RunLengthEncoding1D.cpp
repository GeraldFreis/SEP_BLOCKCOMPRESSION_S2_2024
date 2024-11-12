#include <iostream>
#include <map>
#include <string>
#include <vector>

/// @brief this file contains the first run length encoding algorithm that iterates across the x dimension
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


class Compression {
    // class to handle compressing the input
    private:
        std::vector<std::vector<std::string>> blocks;
        int *headers;
        std::map<char, std::string> tag_table;
        std::vector<std::vector<std::vector<bool>>> visited;

    public:
        Compression(std::vector<std::vector<std::string>> blocks, int *headers, std::map<char, std::string> table);
        void runLengthEncoding();
};

Compression::Compression(std::vector<std::vector<std::string>> blocks_array, int* headers_array, std::map<char, std::string> table) {
  blocks = blocks_array;
  headers = headers_array;
  tag_table = table;
  // Initialize visited array with false
  visited.resize(headers[2], std::vector<std::vector<bool>>(headers[1], std::vector<bool>(headers[0], false)));
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

// main function to run this file
int main () {
    // first we need to read in the data
    Input inp;
    inp.readIn();

    Compression run_length_encoding_trial(inp.getBlocks(), inp.getHeaders(), inp.getTagTable());
    run_length_encoding_trial.runLengthEncoding();
} 

