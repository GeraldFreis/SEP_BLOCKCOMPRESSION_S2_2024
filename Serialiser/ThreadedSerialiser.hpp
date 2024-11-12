#ifndef THREADEDSERIALISER_H
#define THREADEDSERIALISER_H
#include <pthread.h>
#include <unistd.h>

#include <atomic>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <iostream>
#include <thread>

#include "../Input/BasicInput.h"
struct CompressedBlock
{
  uint16_t x_pos, y_pos, z_pos;
  uint8_t x_size, y_size, z_size;
  uint8_t tag;
};
// Define buffer size for serialiser
#ifndef SERIALISER_BUFFER_SIZE
#define SERIALISER_BUFFER_SIZE 1024 // Size of the active buffers
#endif

#ifndef SERIALISER_MAX_LINE_SIZE
#define SERIALISER_MAX_LINE_SIZE 50 // Max size of a single serialized block
#endif

void thread_flush(std::atomic<char> **buffer, size_t *size)
{
  while (*size !=
         SIZE_MAX)
  {             // Loop until size is marked as SIZE_MAX (signal to stop)
    asm("nop"); // Prevent CPU hanging (acts as a no-op)
    if (*buffer != nullptr)
    { // If buffer contains data
      // Flush the atomic buffer (assume safe cast to char*)
      std::cout.write(reinterpret_cast<char *>(*buffer), *size);
      *buffer = nullptr; // Mark buffer as empty
    }
  }
}

class Serialiser
{
private:
  size_t offset = 0; // Current write position in active buffer
  std::atomic<char> *active_buffer =
      nullptr;               // Pointer to the buffer being written to
  size_t writing_offset = 0; // Amount of data to be flushed
  std::atomic<char> *writing_buffer =
      nullptr; // Buffer being flushed in the background
  // Two buffers to alternate between for double-buffering
  std::atomic<char> buffer1[SERIALISER_BUFFER_SIZE];
  std::atomic<char> buffer2[SERIALISER_BUFFER_SIZE];

public:
  // Constructor: initializes buffers and starts a thread for flushing data
  Serialiser()
  {
    active_buffer = buffer1; // Initially use buffer1 as active buffer
    // Start a separate thread to handle buffer flushing
    std::thread trd =
        std::thread(thread_flush, &writing_buffer, &writing_offset);
    trd.detach(); // Detach thread to run independently
  }

  // Destructor: signals thread to stop
  ~Serialiser()
  {
    writing_offset = SIZE_MAX;
    if (offset > 0)
    {
      std::cout.write(reinterpret_cast<char *>(active_buffer), offset);
    }
  }

  // Writes a block's details into the active buffer and flushes if necessary
  void write_block_threaded(Input *input, CompressedBlock &block)
  {

    // Serialize the block fields (x, y, z, sizes) as comma-separated values
    offset += intToBuffer(active_buffer + offset, block.x_pos);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.y_pos);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.z_pos);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.x_size);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.y_size);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.z_size);
    active_buffer[offset++] = ',';

    // Get the string tag label and copy it into the active buffer
    char *label = input->get_tag_label(block.tag);
    while (*label != 0)
    {
      active_buffer[offset++] = *label;
      label++;
    }
    active_buffer[offset++] = '\n'; // Add newline at the end
    if (offset + SERIALISER_MAX_LINE_SIZE >= SERIALISER_BUFFER_SIZE)
    {
      flush();
    }
  }

  void write_block_standard(Input *input, CompressedBlock &block)
  {
    // Serialize the block fields (x, y, z, sizes) as comma-separated values
    offset += intToBuffer(active_buffer + offset, block.x_pos);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.y_pos);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.z_pos);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.x_size);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.y_size);
    active_buffer[offset++] = ',';
    offset += intToBuffer(active_buffer + offset, block.z_size);
    active_buffer[offset++] = ',';

    // Get the string tag label and copy it into the active buffer
    char *label = input->get_tag_label(block.tag);
    while (*label != 0)
    {
      active_buffer[offset++] = *label;
      label++;
    }
    active_buffer[offset++] = '\n'; // Add newline at the end
    if (offset + SERIALISER_MAX_LINE_SIZE >= SERIALISER_BUFFER_SIZE)
    {
      std::cout.write(reinterpret_cast<char *>(active_buffer), offset);
      offset = 0;
    }
  }

  // Flushes the active buffer by switching to the other buffer
  inline void flush()
  {
    // Check if thread is writing, if it is then wait until it is done
    while (writing_buffer != nullptr)
    {
      asm("nop");
    }
    writing_offset = offset;
    writing_buffer = active_buffer;
    offset = 0;

    // Switch buffers
    active_buffer = (active_buffer == buffer1) ? buffer2 : buffer1;
  }

  // Converts an integer (up to 5 digits) to its string representation in the
  // buffer
  static inline size_t intToBuffer(std::atomic<char> *buffer, uint16_t value)
  {
    std::atomic<char> *start = buffer; // Keep track of the starting position

    // Different cases for different integer ranges to minimize divisions
    if (value < 10)
    {
      *buffer++ = '0' + value;
      return 1;
    }
    else if (value < 100)
    {
      *buffer++ = '0' + (value / 10);
      *buffer++ = '0' + (value % 10);
      return 2;
    }
    else if (value < 1000)
    {
      *buffer++ = '0' + (value / 100);
      value %= 100;
      *buffer++ = '0' + (value / 10);
      *buffer++ = '0' + (value % 10);
      return 3;
    }
    else if (value < 10000)
    {
      *buffer++ = '0' + (value / 1000);
      value %= 1000;
      *buffer++ = '0' + (value / 100);
      value %= 100;
      *buffer++ = '0' + (value / 10);
      *buffer++ = '0' + (value % 10);
      return 4;
    }
    else
    {
      *buffer++ = '0' + (value / 10000);
      value %= 10000;
      *buffer++ = '0' + (value / 1000);
      value %= 1000;
      *buffer++ = '0' + (value / 100);
      value %= 100;
      *buffer++ = '0' + (value / 10);
      *buffer++ = '0' + (value % 10);
      return 5;
    }
  }
};

#endif
