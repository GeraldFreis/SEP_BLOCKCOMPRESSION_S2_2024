#include <iostream>
#include <map>
#include <string>
#include <vector>

#ifndef COMPRESSION
#define COMPRESSION

struct CompressedBlock {
    int x, y, z;
    int x_len, y_len, z_len;
    char label;
} ;

class Compression {
    // class to handle compressing the input
    private:
        std::vector<std::vector<std::string>> blocks;
        int *headers;
        std::map<char, std::string> tag_table;
        std::vector<std::vector<std::vector<bool>>> visited;

        std::vector<CompressedBlock> compressedBlocks;

        // Helper function to ensure cursor doesn't exceed boundaries
        bool withinBounds(int x, int y, int z, int px, int py, int pz) {
            return x < px && y < py && z < pz;
        }


    void compressBlock(int start_x, int start_y, int start_z);
    bool checkRowEqual(int z, int y, int x, int x_len, char label);
    bool checkSliceEqual(int z, int y, int x, int x_len, int y_len, char label);
    void markBlockProcessed(int x, int y, int z, int x_len, int y_len, int z_len);

    public:
        Compression(std::vector<std::vector<std::string>> blocks, int *headers, std::map<char, std::string> table);
        void compress();

};

#endif