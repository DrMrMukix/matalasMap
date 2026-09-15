use crate::core::types::{WORLD_HEIGHT, WORLD_WIDTH};
use crate::political::country::Country;
use flate2::read::GzDecoder;
use flate2::write::GzEncoder;
use flate2::Compression;
use serde::{Deserialize, Serialize};
use std::fs::File;
use std::io::{BufReader, BufWriter, Read, Write};
use std::path::Path;

#[derive(Serialize, Deserialize)]
pub struct WorldSaveHeader {
    pub version: u32,
    pub name: String,
    pub created_at: u64,
    pub modified_at: u64,
    pub width: u32,
    pub height: u32,
    pub country_count: usize,
    pub thumbnail_png: Vec<u8>,
}

#[derive(Serialize, Deserialize)]
pub struct WorldSaveFile {
    pub header: WorldSaveHeader,
    pub countries: Vec<Country>,
    pub terrain_compressed: Vec<u8>,
    pub political_compressed: Vec<u8>,
}

pub fn compress_bytes(data: &[u8]) -> std::io::Result<Vec<u8>> {
    let mut encoder = GzEncoder::new(Vec::new(), Compression::fast());
    encoder.write_all(data)?;
    encoder.finish()
}

pub fn decompress_bytes(compressed: &[u8]) -> std::io::Result<Vec<u8>> {
    let mut decoder = GzDecoder::new(compressed);
    let mut out = Vec::new();
    decoder.read_to_end(&mut out)?;
    Ok(out)
}

pub fn generate_thumbnail(
    terrain: &[u8],
    political: &[u16],
    countries: &[Country],
    out_w: u32,
    out_h: u32,
) -> Vec<u8> {
    let step_x = WORLD_WIDTH / out_w;
    let step_y = WORLD_HEIGHT / out_h;

    let mut img = image::RgbaImage::new(out_w, out_h);

    for py in 0..out_h {
        for px in 0..out_w {
            let sx = px * step_x;
            let sy = py * step_y;
            let idx = (sy * WORLD_WIDTH + sx) as usize;

            let is_land = if idx < terrain.len() { terrain[idx] == 1 } else { false };
            let country_id = if idx < political.len() { political[idx] } else { 0 };

            let rgba = if country_id > 0 {
                // Country color
                if let Some(c) = countries.iter().find(|c| c.id == country_id) {
                    image::Rgba([c.color.r, c.color.g, c.color.b, 255])
                } else {
                    image::Rgba([180, 100, 180, 255])
                }
            } else if is_land {
                // Base land color #4E9A51
                image::Rgba([78, 154, 81, 255])
            } else {
                // Ocean #244F75
                image::Rgba([36, 79, 117, 255])
            };

            img.put_pixel(px, py, rgba);
        }
    }

    let mut png_bytes = Vec::new();
    let mut cursor = std::io::Cursor::new(&mut png_bytes);
    let _ = img.write_to(&mut cursor, image::ImageFormat::Png);
    png_bytes
}

pub fn save_world_to_path(
    path: &Path,
    name: &str,
    terrain: &[u8],
    political: &[u16],
    countries: &[Country],
) -> Result<(), Box<dyn std::error::Error>> {
    let now = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)?
        .as_secs();

    let thumbnail_png = generate_thumbnail(terrain, political, countries, 512, 256);

    let terrain_compressed = compress_bytes(terrain)?;

    // Convert u16 political data to little-endian bytes for compression
    let mut pol_raw_bytes = Vec::with_capacity(political.len() * 2);
    for &id in political {
        pol_raw_bytes.extend_from_slice(&id.to_le_bytes());
    }
    let political_compressed = compress_bytes(&pol_raw_bytes)?;

    let save = WorldSaveFile {
        header: WorldSaveHeader {
            version: 1,
            name: name.to_string(),
            created_at: now,
            modified_at: now,
            width: WORLD_WIDTH,
            height: WORLD_HEIGHT,
            country_count: countries.len(),
            thumbnail_png,
        },
        countries: countries.to_vec(),
        terrain_compressed,
        political_compressed,
    };

    let file = File::create(path)?;
    let writer = BufWriter::new(file);
    bincode_like_json_write(writer, &save)?;

    Ok(())
}

fn bincode_like_json_write<W: Write>(writer: W, save: &WorldSaveFile) -> Result<(), Box<dyn std::error::Error>> {
    serde_json::to_writer(writer, save)?;
    Ok(())
}

pub fn load_world_header_only(path: &Path) -> Result<WorldSaveHeader, Box<dyn std::error::Error>> {
    // For fast library listing, read and parse only the header
    let file = File::open(path)?;
    let reader = BufReader::new(file);
    // serde_json can parse streamed structures
    let save: WorldSaveFile = serde_json::from_reader(reader)?;
    Ok(save.header)
}

pub fn load_world_from_bytes(
    bytes: &[u8],
) -> Result<(WorldSaveHeader, Vec<Country>, Vec<u8>, Vec<u16>), Box<dyn std::error::Error>> {
    let reader = std::io::Cursor::new(bytes);
    let save: WorldSaveFile = serde_json::from_reader(reader)?;

    let terrain = decompress_bytes(&save.terrain_compressed)?;

    let pol_raw = decompress_bytes(&save.political_compressed)?;
    let mut political = Vec::with_capacity(pol_raw.len() / 2);
    for chunk in pol_raw.chunks_exact(2) {
        political.push(u16::from_le_bytes([chunk[0], chunk[1]]));
    }

    Ok((save.header, save.countries, terrain, political))
}

pub fn load_world_from_path(
    path: &Path,
) -> Result<(WorldSaveHeader, Vec<Country>, Vec<u8>, Vec<u16>), Box<dyn std::error::Error>> {
    let bytes = std::fs::read(path)?;
    load_world_from_bytes(&bytes)
}
