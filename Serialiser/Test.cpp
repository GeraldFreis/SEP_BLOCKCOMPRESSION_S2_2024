
#include <iostream>

#include "../Input/BasicInput.h"
#include "ThreadedSerialiser.hpp"
#define SERIALISER_BUFFER_SIZE 1024
#define SERIALISER_MAX_LINE_SIZE 50

int main(int argc, char *argv[])
{
  BasicInput input;
  Serialiser serialiser;
  auto sizes = input.get_block_sizes();

  // Allocate memory for the data
  uint8_t *data =
      (uint8_t *)malloc(sizes.count_x * sizes.count_y * sizes.chunk_z);

  // Loop through each z-layer and process blocks
  for (uint16_t z_layer = 0; z_layer < sizes.count_z;
       z_layer += sizes.chunk_z)
  {
    input.get_z_layer(data); // Get a chunk of z-layer data
    uint8_t *data_index = data;

    // Loop through the chunk and write blocks to the serialiser
    for (uint16_t z = 0; z < sizes.chunk_z; z++)
    {
      for (uint16_t y = 0; y < sizes.count_y; y++)
      {
        for (uint16_t x = 0; x < sizes.count_x; x++)
        {
          CompressedBlock block;
          block.tag = *data_index++;
          block.x_pos = x;
          block.y_pos = y;
          block.z_pos = z;
          block.x_size = 1;
          block.y_size = 1;
          block.z_size = 1;
          serialiser.write_block_threaded(&input, block);
        }
      }
    }
  }

  free(data); // Free allocated memory
  return 0;
}
