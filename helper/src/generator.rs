use anyhow::Result;
use rand::rngs::StdRng;
use simdnoise::NoiseBuilder;
use rand::{Rng, SeedableRng};
use rand::distributions::Alphanumeric;

pub struct GeneratorConfig {
    // generation settings
    pub size_x: usize,
    pub size_y: usize,
    pub size_z: usize,
    pub block_x: usize,
    pub block_y: usize,
    pub block_z: usize,
    pub tags_no: usize,
    pub tag_len: usize,
    // noise settings
    pub seed: u32,
    pub octaves: u8,
    pub freq: f32,
    pub gain: f32,
    pub lacunarity: f32,
}

fn generate_tags(seed: u32, max_tags: usize, tag_len: usize) -> (Vec<u8>, Vec<String>) {
    let mut tags: Vec<String> = vec![];
    let mut tag_chars: Vec<u8> = vec![];
    let mut rand_iter = StdRng::seed_from_u64(seed as u64)
        .sample_iter(&Alphanumeric);

    // generate random strings with unique starting letters
    let rand_iter_ref = &mut rand_iter;
    for _ in 0..max_tags {
        let rand_str: String = rand_iter_ref
            .take(tag_len)
            .map(char::from)
            .collect();
        // alphanumric chars can be cast to u8
        let char = rand_str.chars().next().unwrap() as u8;

        if !tag_chars.contains(&char) {
            tags.push(rand_str);
            tag_chars.push(char);
        }
    }

    (tag_chars, tags)
}

fn generate_z_block_layer(config: &GeneratorConfig, tag_chars: &[u8], layer: &mut [u8], z_index: usize) {
    let data = NoiseBuilder::fbm_3d_offset(
        0.0, config.size_x,
        0.0, config.size_y,
        (z_index * config.block_z) as f32, config.block_z
    )
        .with_freq(config.freq)
        .with_octaves(config.octaves)
        .with_gain(config.gain)
        .with_lacunarity(config.lacunarity)
        .with_seed(config.seed as i32)
        // different scaling between z chunk layers doesnt matter
        // -0.01 to prevent max from being out of bounds of tag_chars
        .generate_scaled(0.0, tag_chars.len() as f32 - 0.01);

    for (voxel, value) in layer.iter_mut().zip(data.iter()) {
        // value is always in 0..tag_chars.len()
        let index = *value as usize;
        *voxel = tag_chars[index];
    }
}

pub fn generate(
    config: GeneratorConfig,
    writer: &mut dyn std::io::Write,
) -> Result<()> {
    // write header
    writeln!(writer, "{}, {}, {}, {}, {}, {}",
        config.size_x, config.size_y, config.size_z,
        config.block_x, config.block_y, config.block_z)?;

    // write tags
    let (tag_chars, tags) = generate_tags(config.seed, config.tags_no, config.tag_len);
    for (char, str) in tag_chars.iter().zip(tags.iter()) {
        writeln!(writer, "{}, {}", *char as char, str)?;
    }
    writeln!(writer, "")?;

    // write data
    // need to store a block z layer in memory at a time, as it needs to be printed in 
    // z voxel layers
    let mut block_layer = vec![0u8; config.size_x * config.size_y * config.block_z];
    for z in 0..(config.size_z / config.block_z) {
        generate_z_block_layer(&config, &tag_chars, &mut block_layer, z);

        let mut iter = block_layer.chunks(config.size_x);
        for _ in 0..config.block_z {
            for _ in 0..config.size_y {
                writer.write(iter.next().unwrap())?;
                writeln!(writer, "")?;
            }
            writeln!(writer, "")?;
        }
    }

    Ok(())
}
