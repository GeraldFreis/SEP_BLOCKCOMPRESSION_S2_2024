#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <chrono>
#include <iostream>

#define SIZEX 1024
#define SIZEY 1024
#define SIZEZ 1024
#define RANDOMCOUNT 100000000

static uint8_t* flat_array;
static uint8_t*** multi_array;

// benchmark
template <typename Func>
void benchmark(Func func, const char* text) {
    auto start = std::chrono::high_resolution_clock::now();

    size_t out = func();

    auto end = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> duration = end - start;

    std::cout << text << " took " << duration.count() << " seconds val="
              << out << std::endl;
}

// linear
size_t benchmark_linear_flat(){
    size_t count = 0;
    for (size_t i=0; i < (size_t)SIZEX * SIZEY * SIZEZ; i++) {
        count += flat_array[i];
    }
    return count;
}

size_t benchmark_linear_multi(){
    size_t count = 0;
    for (size_t zi=0; zi < SIZEZ; zi++) {
        for (size_t yi=0; yi < SIZEY; yi++) {
            for (size_t xi=0; xi < SIZEX; xi++) {
                count += multi_array[zi][yi][xi];
            }
        }
    }
    return count;
}

// random
size_t benchmark_random_flat(){
    size_t count = 0;
    for (size_t i=0; i < (size_t)RANDOMCOUNT; i++) {
        size_t x = rand() % SIZEX;
        size_t y = rand() % SIZEY;
        size_t z = rand() % SIZEZ;
        size_t index = z * SIZEZ + y * SIZEY + x;
        count += flat_array[index];
    }
    return count;
}

size_t benchmark_random_multi(){
    size_t count = 0;
    for (size_t i=0; i < (size_t)RANDOMCOUNT; i++) {
        size_t x = rand() % SIZEX;
        size_t y = rand() % SIZEY;
        size_t z = rand() % SIZEZ;
        count += multi_array[z][y][x];
    }
    return count;
}

int main(int argc, char *argv[]) {
    // setup
    flat_array = (uint8_t*)malloc((size_t)SIZEX * SIZEY * SIZEZ);
    for (size_t i=0; i < (size_t)SIZEX * SIZEY * SIZEZ; i++) {
        flat_array[i] = (uint8_t)i;
    }

    multi_array = (uint8_t***)malloc(SIZEZ * sizeof(uint8_t**));
    for (size_t zi=0; zi < SIZEZ; zi++) {
        multi_array[zi] = (uint8_t**)malloc(SIZEY * sizeof(uint8_t*));
        for (size_t yi=0; yi < SIZEY; yi++) {
            multi_array[zi][yi] = (uint8_t*)malloc(SIZEX * sizeof(uint8_t));
            for (size_t xi=0; xi < SIZEX; xi++) {
                multi_array[zi][yi][xi] = (uint8_t)xi;
            }
        }
    }

    benchmark(benchmark_linear_flat, "linear flat");
    benchmark(benchmark_linear_multi, "linear multi");
    benchmark(benchmark_linear_flat, "linear flat");
    benchmark(benchmark_linear_multi, "linear multi");
    benchmark(benchmark_linear_flat, "linear flat");
    benchmark(benchmark_linear_flat, "linear flat");
    benchmark(benchmark_linear_multi, "linear multi");
    benchmark(benchmark_linear_multi, "linear multi");

    benchmark(benchmark_random_multi, "random multi");
    benchmark(benchmark_random_flat, "random flat");
    benchmark(benchmark_random_multi, "random multi");
    benchmark(benchmark_random_flat, "random flat");
    
    return 0;
}
