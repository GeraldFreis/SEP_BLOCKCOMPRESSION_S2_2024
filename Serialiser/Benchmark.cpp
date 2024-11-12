
#include <chrono>
#include <iostream>

#include "../Input/BasicInput.h"
#include "ThreadedSerialiser.h"
#define SERIALISER_BUFFER_SIZE 1024
#define SERIALISER_MAX_LINE_SIZE 50

template <typename Func>
void benchmark(Func function, char* text) {
  auto start = std::chrono::high_resolution_clock::now();
  function();
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double> duration = end - start;
  std::cout << text << " took " << duration.count()
            << " seconds val=" << std::endl;
}

int main(int argc, char* argv[]) {
  BasicInput input;
  Serialiser serialiser;

  // Allocate memory for the data
  auto sizes = input.get_block_sizes();
  uint8_t* data =
      (uint8_t*)malloc(sizes.count_x * sizes.count_y * sizes.chunk_z);

  // Lambda to run the threaded serialisation
  auto benchmark_threaded = [&]() {
    for (uint16_t z_layer = 0; z_layer < sizes.count_z;
         z_layer += sizes.chunk_z) {
      input.get_z_layer(data);  // Get a chunk of z-layer data
      uint8_t* data_index = data;
      for (uint16_t z = 0; z < sizes.chunk_z; z++) {
        for (uint16_t y = 0; y < sizes.count_y; y++) {
          for (uint16_t x = 0; x < sizes.count_x; x++) {
            serialiser.write_block_threaded(&input, x, y, z_layer + z,
                                            *data_index++);
          }
        }
      }
    }
  };

  // Lambda to run the standard serialisation
  auto benchmark_standard = [&]() {
    for (uint16_t z_layer = 0; z_layer < sizes.count_z;
         z_layer += sizes.chunk_z) {
      input.get_z_layer(data);  // Get a chunk of z-layer data
      uint8_t* data_index = data;
      for (uint16_t z = 0; z < sizes.chunk_z; z++) {
        for (uint16_t y = 0; y < sizes.count_y; y++) {
          for (uint16_t x = 0; x < sizes.count_x; x++) {
            serialiser.write_block_standard(&input, x, y, z_layer + z,
                                            *data_index++);
          }
        }
      }
    }
  };

  // Benchmark the threaded serialization
  benchmark(benchmark_threaded, "Threaded serialisation");

  // Benchmark the standard serialization
  benchmark(benchmark_standard, "Standard serialisation");

  free(data);  // Free allocated memory
  return 0;
}