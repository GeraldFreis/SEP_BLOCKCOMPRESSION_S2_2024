# Block compression project for Maptek at UOA (university of adelaide) semester 2 2024 
### Compression Algorithm
Compression.h and Compression.cpp are used for creating the compression algorithm
run_length_encoding.cpp is the first working algorithm with all classes in one cpp for debugging and quick compilation
we also have different variations of run length encoding like RunLengthEncoding3D.cpp which iterates across three dimensions.
We also have LZ77 compression which can be found in the CompressionLZ77 directory.
## Helpers
To run the helpers Rust and Cargo are required. Install Cargo and Rust, instructions can be found [here](https://www.rust-lang.org/tools/install) or summarised below.

```sh
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```

After Cargo and Rust are installed it can be run with the following command. It will take longer the first time as it builds all the dependancies.

```sh
cargo run --release
```

### Generator and Checker
The generator generates test data for profiling, the checker is then used to validate the output of a compression algorithem and aid in finding the points where it fails. Help for both is located [here](helper/readme.md).

### Visualiser
The visualiser can be used to assist debugging the compressors output. Help for the visualiser is located [here](visualiser/readme.md)



### FixedSizeCompression Algorithm
Takes fixed sized large blocks, and checks to see if every small block inside the large block is the same "colour". If it is, then it compresses the entire block. Otherwise, it does not perform any compression on that large block. By default, the large blocks are taken to be the same size as the parent blocks.