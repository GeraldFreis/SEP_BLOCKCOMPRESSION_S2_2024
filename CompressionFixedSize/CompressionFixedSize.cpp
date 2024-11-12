#include "CompressionFixedSize.h"

#include <iostream>

#include "Input.h"

CompressionFixedSize::CompressionFixedSize() {
    /* Read input from stdin and store in _input */
    _input.readIn();

    /* Set large block bounds to be the same as the parent blocks */
    _max_x = _input.parent_x;
    _max_y = _input.parent_y;
    _max_z = _input.parent_z;
}

CompressionFixedSize::CompressionFixedSize(int max_x, int max_y, int max_z) {
    try {
        /* Read input from stdin and store in _input */
        _input.readIn();

        /* Check if the large blocks evenly divide the small blocks */
        if (_input.x_count % _max_x != 0 || _input.y_count % _max_y != 0 ||
            _input.z_count % _max_z != 0) {
            throw new std::exception();
        }

        /* Set large block bounds to the given values */
        _max_x = max_x;
        _max_y = max_y;
        _max_z = max_z;
    } catch (const std::exception& e) {
        std::cerr << e.what()
                  << "Large blocks do not evenly divide block model.\n";
    }
}

void CompressionFixedSize::Compress() {
    int x_count = _input.x_count;
    int y_count = _input.y_count;
    int z_count = _input.z_count;

    /* Determine how many large blocks there are */
    int x = x_count / _max_x;
    int y = y_count / _max_y;
    int z = z_count / _max_z;

    /* loop to determine if the large blocks are uniform */
    for (int i = 0; i < z; i++) {
        for (int j = 0; j < y; j++) {
            for (int k = 0; k < x; k++) {
                /* Call isUniform on the corner of the block */
                if (isUniform(k * _max_x, j * _max_y, i * _max_z)) {
                    /* if is Uniform, output the whole block as compressed */
                    output(k * _max_x, j * _max_y, i * _max_z, true);
                } else {
                    /* in not uniform, output each individual block
                     * (uncompressed) */
                    for (int l = 0; l < _max_z; l++) {
                        for (int m = 0; m < _max_y; m++) {
                            for (int n = 0; n < _max_x; n++) {
                                output(n + _max_x * k, m + _max_y * j,
                                       l + _max_z * i, false);
                            }
                        }
                    }
                }
            }
        }
    }
}

bool CompressionFixedSize::isUniform(int x, int y, int z) {
    /* Colour of the left lower bottom corner block */
    char colour = _input.blocks.at(z).at(y).at(x);

    /* Loop to check if any block is different to colour */
    for (int i = z; i < z + _max_z; i++) {
        for (int j = y; j < y + _max_y; j++) {
            std::string curr_line = _input.blocks.at(i).at(j).substr(x, _max_x);
            if (curr_line.find_first_not_of(colour) != std::string::npos)
                return false;
        }
    }
    return true;
}

void CompressionFixedSize::output(int x, int y, int z, bool uniform) {
    if (uniform) {
        std::cout << x << ", " << y << ", " << z << ", " << _max_x << ", "
                  << _max_y << ", " << _max_z << ", "
                  << _input.getTagTable().at(_input.blocks.at(z).at(y).at(x))
                  << std::endl;
    } else {
        std::cout << x << ", " << y << ", " << z << ", " << 1 << ", " << 1
                  << ", " << 1 << ", "
                  << _input.getTagTable().at(_input.blocks.at(z).at(y).at(x))
                  << std::endl;
    }
}
