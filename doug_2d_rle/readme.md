# Compression Algorithem
Uses the previous 1d bit manipulation RLE version, but compares current rows mask with the next 
row anded with the mask, until a match isnt found. The current fastest version is `fast_2d` but the fastest submitted is `fast_2d_single`.

# fast_2d_single
Single threaded version.

# fast_2d_msingle
Single threaded version with a seperate thread for writing to stdout.

# fast_2d
Multithreaded version using spin lock atomics and lock free queues. The lock free queue `concurrentque.h` is a 3rd party library. Suffers from "works on my laptop but not in production".

# fast_2d_mutex
Multithreaded version using mutex locks, checker fails but needs more checking.
