#ifndef COMPRESSIONFIXEDSIZE
#define COMPRESSIONFIXEDSIZE

#include "Input.h"

class CompressionFixedSize {
   private:
    /* Object to read and store block data */
    Input _input;
    /* Large block size */
    int _max_x;
    int _max_y;
    int _max_z;

   public:
    /* NOTE: input is read when this class is created, so it does not need to be
     * done in compress() or in any main file. */
    /* DEFAULT CONSTRUCTOR */
    CompressionFixedSize();
    /* Constructor to define specific block size. */
    CompressionFixedSize(int max_x, int max_y, int max_z);

    /* Main function which makes calls to recieve input, runs the algorithm, and
     * outputs the result */
    void Compress();
    /* Checks if every block inside the given bounds is the same "colour"
     * @param x The x value of the left lower bottom corner of the large block.
     * This will be equal to the smallest x coordinate in the block.
     * @param y The y value of the left lower bottom corner of the large block.
     * This will be equal to the smallest y coordinate in the block.
     * @param z The z value of the left lower bottom corner of the large block.
     * This will be equal to the smallest z coordinate in the block.
     * @returns Boolean value which is true if every block within the bounds is
     * the same and false otherwise.
     */
    bool isUniform(int z, int y, int x);
    /* Outputs a compressed block in the required format
     * @param k 
     */
    void output(int z, int y, int x, bool uniform);
};
#endif