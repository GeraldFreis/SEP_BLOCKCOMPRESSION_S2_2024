use std::{collections::HashMap, fs::File, io::{BufRead, BufReader, Read}, path::PathBuf};
use anyhow::{anyhow, Context, Result};

pub const EMPTY_STRING: String = String::new();

#[derive(Debug)]
pub struct BaseData {
    pub x_count: usize,
    pub y_count: usize,
    pub z_count: usize,
    pub parent_size_x: usize,
    pub parent_size_y: usize,
    pub parent_size_z: usize,
    pub tags: HashMap<u8, String>,
    pub data: Box<[u8]>,
}

#[derive(Debug)]
pub struct CompressedBlock {
    pub x_pos: u16,
    pub y_pos: u16,
    pub z_pos: u16,
    pub x_size: u16,
    pub y_size: u16,
    pub z_size: u16,
    pub tag: u8,
}

impl std::fmt::Display for CompressedBlock {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "Block({}, {}, {}, {}, {}, {})",
            self.x_pos, self.y_pos, self.z_pos,
            self.x_size, self.y_size, self.z_size)
    }
}

pub fn read_base_file(path: PathBuf) -> Result<BaseData> {
    let file = File::open(path)?;
    let mut buf_reader = BufReader::new(file);
    
    // sizes
    let mut line = String::new();
    let _ = buf_reader.read_line(&mut line);
    let mut iter = line.split(',')
        .filter_map(|s| s.trim().parse::<u16>().ok());

    macro_rules! next {
        ($text:expr) => {
            iter.next().context($text)? as usize
        };
    }

    let x_count = next!("no x count");
    let y_count = next!("no y count");
    let z_count = next!("no z count");

    let parent_size_x = next!("no parent x size");
    let parent_size_y = next!("no parent y size");
    let parent_size_z = next!("no parent z size");

    line.clear();

    // tags
    let mut tags = HashMap::new();
    while buf_reader.read_line(&mut line)? > 2 {
        let mut iter = line.chars();

        let index = iter.next().context("no char")? as u8;
        let value = iter.skip_while(|&c| c != ',')
            .skip(1)
            .collect::<String>();
        
        if tags.insert(index, value.trim().to_string()).is_some() {
            return Err(anyhow!("duplicate tag {}", index as char));
        }

        line.clear();
    }

    // data
    let mut byte_iter = buf_reader.bytes()
        .filter(|c| !matches!(c, Ok(b'\n')) && !matches!(c, Ok(b'\r')));
    let mut data = vec![0u8; (x_count * y_count * z_count) as usize].into_boxed_slice();

    for v in data.iter_mut() {
        *v = byte_iter
            .next()
            .expect("not enough data chars")?;
    }

    Ok(BaseData {
        x_count,
        y_count,
        z_count,
        parent_size_x,
        parent_size_y,
        parent_size_z,
        tags,
        data,
    })
}

pub fn read_output_file(path: PathBuf, tags: &HashMap<u8, String>) -> Result<Vec<CompressedBlock>> {
    let file = File::open(path)?;
    let mut buf_reader = BufReader::new(file);

    let mut compressed_blocks = vec![];
    let mut line = String::new();

    // inverted hashmap for lookup
    let mut tag_lookup = HashMap::new();
    for (&index, value) in tags.iter() {
        tag_lookup.insert(value.as_str(), index);
    }

    while buf_reader.read_line(&mut line).is_ok() {
        let mut iter = line.split(',')
            .map(|s| s.trim());
        let mut num_iter = (&mut iter)
            .filter_map(|s| s.parse::<u16>().ok());

        macro_rules! next {
            ($text:expr) => {
                num_iter.next().with_context(||$text)?
            };
        }

        let x_pos = match num_iter.next() {
            Some(v) => v,
            None => break,
        };
        let y_pos = next!("no y count");
        let z_pos = next!("no z count");

        let x_size = next!("no parent x size");
        let y_size = next!("no parent y size");
        let z_size = next!("no parent z size");

        let tag_str = iter.next().with_context(||"no tag")?;
        let tag = *tag_lookup.get(tag_str)
            .with_context(|| format!("no matching tag to label {}", tag_str))?;

        compressed_blocks.push(CompressedBlock {
            x_pos,
            y_pos,
            z_pos,
            x_size,
            y_size,
            z_size,
            tag,
        });

        line.clear();
    }

    Ok(compressed_blocks)
}
