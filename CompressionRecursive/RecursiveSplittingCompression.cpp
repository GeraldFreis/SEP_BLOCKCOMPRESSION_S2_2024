#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifndef Compressor

class Input {
 private:
  int x_count, y_count, z_count, parent_x, parent_y, parent_z;
  std::map<char, std::string> tag_table;
  std::vector<std::vector<std::string>> blocks;

 public:
  Input();
  void readIn();
  void blocksInit();
  void printBlocks();
  void printTable();
  std::vector<std::vector<std::string>> getBlocks();
  int getParentX() { return parent_x; }
  int getParentY() { return parent_y; }
  int getParentZ() { return parent_z; }
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

  std::getline(std::cin, str);

  while (str.length() > 3) {
    char tag = str.at(0);
    std::string label = str.substr(3);
    tag_table.insert({tag, label});
    std::getline(std::cin, str);
  }

  blocksInit();

  for (int i = 0; i < z_count; i++) {
    for (int j = 0; j < y_count; j++) {
      std::getline(std::cin, str);
      if (str.compare("") == 0) continue;
      blocks.at(i).at(j) = str;
    }
    std::getline(std::cin, str);
  }

  // changing it so that the last blocks become the first, to make sure that the
  // lower left bottom corner is at 0,0,0 std::vector<std::vector<std::string>>
  // new_blocks; int i_count = 0; int j_count = 0; for (int i = z_count -1 ; i
  // >= 0; i--) {
  //     for (int j = y_count - 1; j >= 0; j--) {

  //         new_blocks.at(i_count).at(j_count) = blocks.at(i).at(j);
  //         j_count++;
  //     }
  //     i_count++;
  // }
  // blocks = new_blocks;
}

void Input::blocksInit() {
  if (x_count <= 0 || y_count <= 0 || z_count <= 0) {
    std::cerr << "dimensions are non-natural: x: " << x_count
              << ", y: " << y_count << ", z: " << z_count << std::endl;
    return;
  }

  std::string line(x_count, ' ');
  std::vector<std::string> slice(y_count, line);
  blocks.resize(z_count, slice);
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

std::vector<std::vector<std::string>> Input::getBlocks() { return blocks; }

class Compressor {
 private:
  Input input;
  std::vector<std::string> output;
  std::vector<std::vector<std::string>> blocks_array;

  bool isUniform(int x_start, int y_start, int z_start, int x_size, int y_size,
                 int z_size) {
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

  void compressBlock(int x_start, int y_start, int z_start, int x_size,
                     int y_size, int z_size, char label) {
    std::string line = std::to_string(x_start) + "," + std::to_string(y_start) +
                       "," + std::to_string(z_start) + ",";
    line += std::to_string(x_size) + "," + std::to_string(y_size) + "," +
            std::to_string(z_size) + "," + label;
    // line += label;

    std::cout << line << std::endl;
  }

  // void processParentBlock(int x_start, int y_start, int z_start, int x_size,
  // int y_size, int z_size) {
  //     if (x_size == 0 || y_size == 0 || z_size == 0) return;

  //     if (isUniform(x_start, y_start, z_start, x_size, y_size, z_size)) {
  //         char label = blocks_array[z_start][y_start][x_start];
  //         compressBlock(x_start, y_start, z_start, x_size, y_size, z_size,
  //         label);
  //     }
  //     else {
  //         int mid_x = std::min(x_size / 2, x_size);
  //         int mid_y = std::min(y_size / 2, y_size);
  //         int mid_z = std::min(z_size / 2, z_size);

  //         processParentBlock(x_start, y_start, z_start, mid_x, mid_y, mid_z);
  //         processParentBlock(x_start + mid_x, y_start, z_start, x_size -
  //         mid_x, mid_y, mid_z); processParentBlock(x_start, y_start + mid_y,
  //         z_start, mid_x, y_size - mid_y, mid_z); processParentBlock(x_start
  //         + mid_x, y_start + mid_y, z_start, x_size - mid_x, y_size - mid_y,
  //         mid_z);

  //         processParentBlock(x_start, y_start, z_start + mid_z, mid_x, mid_y,
  //         z_size - mid_z); processParentBlock(x_start + mid_x, y_start,
  //         z_start + mid_z, x_size - mid_x, mid_y, z_size - mid_z);
  //         processParentBlock(x_start, y_start + mid_y, z_start + mid_z,
  //         mid_x, y_size - mid_y, z_size - mid_z); processParentBlock(x_start
  //         + mid_x, y_start + mid_y, z_start + mid_z, x_size - mid_x, y_size -
  //         mid_y, z_size - mid_z);
  //     }
  // }
  void processParentBlock(int x_start, int y_start, int z_start, int x_size,
                          int y_size, int z_size) {
    // Enforce maximum dimensions
    x_size = std::min(x_size, input.getParentX());
    y_size = std::min(y_size, input.getParentY());
    z_size = std::min(z_size, input.getParentZ());

    if (x_size <= 0 || y_size <= 0 || z_size <= 0) return;

    if (isUniform(x_start, y_start, z_start, x_size, y_size, z_size)) {
      char label = blocks_array[z_start][y_start][x_start];
      compressBlock(x_start, y_start, z_start, x_size, y_size, z_size, label);
    } else {
      int mid_x = std::min(x_size / 2, x_size);
      int mid_y = std::min(y_size / 2, y_size);
      int mid_z = std::min(z_size / 2, z_size);

      processParentBlock(x_start, y_start, z_start, mid_x, mid_y, mid_z);
      processParentBlock(x_start + mid_x, y_start, z_start, x_size - mid_x,
                         mid_y, mid_z);
      processParentBlock(x_start, y_start + mid_y, z_start, mid_x,
                         y_size - mid_y, mid_z);
      processParentBlock(x_start + mid_x, y_start + mid_y, z_start,
                         x_size - mid_x, y_size - mid_y, mid_z);

      processParentBlock(x_start, y_start, z_start + mid_z, mid_x, mid_y,
                         z_size - mid_z);
      processParentBlock(x_start + mid_x, y_start, z_start + mid_z,
                         x_size - mid_x, mid_y, z_size - mid_z);
      processParentBlock(x_start, y_start + mid_y, z_start + mid_z, mid_x,
                         y_size - mid_y, z_size - mid_z);
      processParentBlock(x_start + mid_x, y_start + mid_y, z_start + mid_z,
                         x_size - mid_x, y_size - mid_y, z_size - mid_z);
    }
  }

 public:
  void compress() {
    input.readIn();
    blocks_array = input.getBlocks();

    int x_count = blocks_array[0][0].size();
    int y_count = blocks_array[0].size();
    int z_count = blocks_array.size();

    int parent_x = input.getParentX();
    int parent_y = input.getParentY();
    int parent_z = input.getParentZ();

    for (int z = 0; z < z_count; z += parent_z) {
      for (int y = 0; y < y_count; y += parent_y) {
        for (int x = 0; x < x_count; x += parent_x) {
          int current_x_size = std::min(parent_x, x_count - x);
          int current_y_size = std::min(parent_y, y_count - y);
          int current_z_size = std::min(parent_z, z_count - z);

          processParentBlock(x, y, z, current_x_size, current_y_size,
                             current_z_size);
        }
      }
    }
  }
};

#endif

int main() {
  Compressor compressor;
  compressor.compress();
  return 0;
}