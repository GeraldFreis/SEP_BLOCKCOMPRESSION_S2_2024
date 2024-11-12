# Checker
The checker will check a compressed output against its base file, it will notify which blocks are overflowing, missing, or overlapping. To use you just need to pass in the base file and compressed ouput.
```sh
# Basic Usage
cargo run --release -- check <BaseFile> <OutputFile>

# For help
cargo run --release -- check --help
```

# Generator
The generator generates large test data for performance testing. The size of the world and size of blocks can be specified as well as the number of tags and other settings for the noise function. By default data will be written to stdout, to change this pass an output file path with `--output-file`.

```sh
# To see all arguments
cargo run --release -- gen --help

# Basic Usage
cargo run --release -- gen 12 12 9 4 4 3

# Writting output to a file
cargo run --release -- gen 100 100 80 10 10 10 -o output.txt
```
