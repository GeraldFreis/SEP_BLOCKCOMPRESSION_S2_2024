use std::{ffi::OsString, fs::File, path::PathBuf};
use clap::Parser;
use anyhow::{anyhow, ensure, Result};
use helper::{self, BaseData, CompressedBlock};

mod generator;


#[derive(Parser)]
enum Args {
    /// Checks if a compression is correct
    Check {
        /// The input to compress
        base: PathBuf,
        /// The output from the compressor
        output: PathBuf,
    },
    /// Generate a large dataset to test with
    Gen {
        size_x: u16,
        size_y: u16,
        size_z: u16,
        block_x: u16,
        block_y: u16,
        block_z: u16,
        /// Maximum number of tags to use
        #[arg(short, long, default_value="5")]
        tags_no: u8,
        /// Length of tag to generate
        #[arg(short, long, default_value="7")]
        tag_len: u8,
        /// Seed to use for generation
        #[arg(short, long, default_value="0")]
        seed: u32,
        /// Frequency to use for the noise generator
        #[arg(short, long, default_value="0.005")]
        freq: f32,
        /// Lacunarity to use for the noise generator
        #[arg(short, long, default_value="0.001")]
        lacunarity: f32,
        /// File to write to
        #[arg(short='o', long="output-file")]
        file: Option<OsString>,
    },
}

fn main() -> Result<()> {
    match Args::parse() {
        Args::Check { base, output } => {
            let base_data = helper::read_base_file(base)?;
            let compressed_blocks = helper::read_output_file(output, &base_data.tags)?;
            check_compression(base_data, compressed_blocks)
        },
        Args::Gen { 
            size_x, size_y, size_z, block_x, block_y, block_z,
            seed, freq, lacunarity,
            tags_no, tag_len, file
        } => {
            ensure!(size_x % block_x == 0, "Block x size is not multiple of world size");
            ensure!(size_y % block_y == 0, "Block y size is not multiple of world size");
            ensure!(size_z % block_z == 0, "Block z size is not multiple of world size");

            let config = generator::GeneratorConfig {
                size_x: size_x as usize,
                size_y: size_y as usize,
                size_z: size_z as usize,
                block_x: block_x as usize,
                block_y: block_y as usize,
                block_z: block_z as usize,
                tags_no: tags_no as usize,
                tag_len: tag_len as usize,
                octaves: 1,
                freq,
                gain: 1.0,
                lacunarity,
                seed,
            };

            let mut writer: Box<dyn std::io::Write> = if let Some(path) = file {
                let file = File::create(path)?;
                let buf_writter = std::io::BufWriter::new(file);
                Box::new(buf_writter)
            } else {
                Box::new(std::io::stdout())
            };

            generator::generate(config, &mut writer)?;
            Ok(())
        }
    }
}

fn check_compression(mut base_data: BaseData, compressed_blocks: Vec<CompressedBlock>) -> Result<()> {
    let mut failed = false;
    'block_loop: for block in compressed_blocks {

        // check bounds
        if block.x_pos as usize % base_data.parent_size_x + block.x_size as usize > base_data.parent_size_x {
            eprintln!("{} has x overflowing parent chunk", block);
            failed = true;
            continue;
        }
        if block.y_pos as usize % base_data.parent_size_y + block.y_size as usize > base_data.parent_size_y {
            eprintln!("{} has y overflowing parent chunk", block);
            failed = true;
            continue;
        }
        if block.z_pos as usize % base_data.parent_size_z + block.z_size as usize > base_data.parent_size_z {
            eprintln!("{} has z overflowing parent chunk", block);
            failed = true;
            continue;
        }

        // get starting index for block
        let mut index = block.z_pos as usize * base_data.y_count;
        index += block.y_pos as usize;
        index *= base_data.x_count;
        index += block.x_pos as usize;

        // check and clear voxels
        for z in block.z_pos..(block.z_pos + block.z_size) {
            for y in block.y_pos..(block.y_pos + block.y_size) {
                for x in block.x_pos..(block.x_pos + block.x_size) {
                    let tag = base_data.data[index];
                    if tag == 0 {
                        eprintln!("{} overlaps with another block at ({x}, {y}, {z})", block);
                        failed = true;
                        continue 'block_loop;
                    }
                    if tag != block.tag {
                        eprintln!("{} includes voxel with incorrect tag {} at ({x}, {y}, {z}), should be {}",
                            block, block.tag as char, tag as char);
                        failed = true;
                        continue 'block_loop;
                    }
                    // set voxel to 0 to detect overlapping
                    base_data.data[index] = 0;

                    index += 1;
                }
                index += base_data.x_count - block.x_size as usize;
            }
            index += (base_data.y_count - block.y_size as usize) * base_data.x_count;
        }
    }

    // check all block are accounted for
    for (mut i, &v) in base_data.data.iter().enumerate() {
        if v != 0 {
            let x = i % base_data.x_count;
            i = (i - x) / base_data.x_count;
            let y = i % base_data.y_count;
            i = (i - y) / base_data.y_count;
            let z = i;

            eprintln!("Voxel ({x}, {y}, {z}) is missing");
            failed = true;
        }
    }

    // return correct status code
    if failed {
        Err(anyhow!("failed"))
    } else {
        println!("Correct");
        Ok(())
    }
}
