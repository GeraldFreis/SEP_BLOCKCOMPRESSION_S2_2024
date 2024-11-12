#include <vector>
#include <map>
#include <string>


class Compression {
    // class to handle compressing the input
    private:
        std::vector<std::vector<std::string>> blocks;
        int *headers;
        std::map<char, std::string> tag_table;
    public:
        Compression(std::vector<std::vector<std::string>> blocks, int *headers, std::map<char, std::string> table);
        void runLengthEncoding();
};

