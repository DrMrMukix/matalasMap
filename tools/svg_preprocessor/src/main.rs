use std::fs::{self, File};
use std::io::BufWriter;
use std::path::Path;

const TARGET_WIDTH: u32 = 8192;
const TARGET_HEIGHT: u32 = 4096;

// Flat colors as requested:
// Water: Pure flat ocean blue #244F75 (R: 36, G: 79, B: 117, A: 255)
const WATER_COLOR: [u8; 4] = [36, 79, 117, 255];
// Land: Pure flat land green #4E9A51 (R: 78, G: 154, B: 81, A: 255)
const LAND_COLOR: [u8; 4] = [78, 154, 81, 255];

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let svg_path = Path::new("Equirectangular_projection_world_map_without_borders.svg");
    if !svg_path.exists() {
        eprintln!("Error: SVG not found at {:?}", svg_path);
        std::process::exit(1);
    }

    println!("Reading SVG from {:?}...", svg_path);
    let svg_data = fs::read(svg_path)?;

    let opt = usvg::Options::default();
    let tree = usvg::Tree::from_data(&svg_data, &opt)?;

    let svg_size = tree.size();
    println!("Original SVG viewBox size: {}x{}", svg_size.width(), svg_size.height());

    let scale_x = TARGET_WIDTH as f32 / svg_size.width();
    let scale_y = TARGET_HEIGHT as f32 / svg_size.height();
    println!("Target resolution: {}x{}", TARGET_WIDTH, TARGET_HEIGHT);
    println!("Scale factors: x={}, y={}", scale_x, scale_y);

    let transform = tiny_skia::Transform::from_scale(scale_x, scale_y);

    println!("Allocating {}x{} raster pixmap...", TARGET_WIDTH, TARGET_HEIGHT);
    let mut pixmap = tiny_skia::Pixmap::new(TARGET_WIDTH, TARGET_HEIGHT)
        .ok_or("Failed to allocate pixmap")?;

    println!("Rendering SVG with resvg...");
    resvg::render(&tree, transform, &mut pixmap.as_mut());

    println!("Generating flat 8K PNG image (Water: {:?}, Land: {:?})...", WATER_COLOR, LAND_COLOR);
    let total_pixels = (TARGET_WIDTH * TARGET_HEIGHT) as usize;
    let mut output_rgba = vec![0u8; total_pixels * 4];

    let rendered_bytes = pixmap.data();
    let mut land_count = 0usize;

    for i in 0..total_pixels {
        let alpha = rendered_bytes[i * 4 + 3];
        let out_idx = i * 4;

        if alpha > 50 {
            output_rgba[out_idx..out_idx + 4].copy_from_slice(&LAND_COLOR);
            land_count += 1;
        } else {
            output_rgba[out_idx..out_idx + 4].copy_from_slice(&WATER_COLOR);
        }
    }

    println!(
        "Processed {} pixels: {} land ({:.2}%), {} water ({:.2}%)",
        total_pixels,
        land_count,
        (land_count as f64 / total_pixels as f64) * 100.0,
        total_pixels - land_count,
        ((total_pixels - land_count) as f64 / total_pixels as f64) * 100.0
    );

    // Ensure assets/maps directory exists
    let maps_dir = Path::new("assets/maps");
    fs::create_dir_all(maps_dir)?;

    let png_path = maps_dir.join("earth_terrain_8192x4096.png");
    println!("Encoding and saving 8K flat PNG to {:?}...", png_path);

    let file = File::create(&png_path)?;
    let writer = BufWriter::new(file);

    let encoder = image::codecs::png::PngEncoder::new_with_quality(
        writer,
        image::codecs::png::CompressionType::Fast,
        image::codecs::png::FilterType::Sub,
    );

    image::ImageEncoder::write_image(
        encoder,
        &output_rgba,
        TARGET_WIDTH,
        TARGET_HEIGHT,
        image::ExtendedColorType::Rgba8,
    )?;

    let file_size = fs::metadata(&png_path)?.len();
    println!("SUCCESS! Saved 8K PNG {:?} ({:.2} MB)", png_path, file_size as f64 / (1024.0 * 1024.0));

    // Also create thumbnail preview (512x256)
    let preview_w = 512u32;
    let preview_h = 256u32;
    let mut preview_img = image::RgbaImage::new(preview_w, preview_h);
    let step_x = TARGET_WIDTH / preview_w;
    let step_y = TARGET_HEIGHT / preview_h;

    for py in 0..preview_h {
        for px in 0..preview_w {
            let src_x = px * step_x;
            let src_y = py * step_y;
            let src_idx = ((src_y * TARGET_WIDTH + src_x) * 4) as usize;
            preview_img.put_pixel(
                px,
                py,
                image::Rgba([
                    output_rgba[src_idx],
                    output_rgba[src_idx + 1],
                    output_rgba[src_idx + 2],
                    output_rgba[src_idx + 3],
                ]),
            );
        }
    }

    let preview_path = maps_dir.join("earth_preview_512x256.png");
    preview_img.save(&preview_path)?;
    println!("Saved preview PNG to {:?}", preview_path);

    Ok(())
}
