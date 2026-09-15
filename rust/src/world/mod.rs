use crate::core::types::{
    ColorRgba, DisplayMode, EditorMode, Rect, ToolType, WORLD_HEIGHT, WORLD_WIDTH,
};
use crate::political::country::{Country, CountryId};
use crate::political::grid::PoliticalGrid;
use crate::save;
use crate::terrain::grid::TerrainGrid;
use crate::undo::UndoManager;
use std::collections::HashMap;
use std::path::Path;

pub const WATER_RGBA: ColorRgba = ColorRgba::new(36, 79, 117, 255);
pub const LAND_RGBA: ColorRgba = ColorRgba::new(78, 154, 81, 255);

pub struct WorldState {
    pub name: String,
    pub width: u32,
    pub height: u32,
    pub terrain: TerrainGrid,
    pub political: PoliticalGrid,
    pub countries: Vec<Country>,
    pub next_country_id: CountryId,
    pub active_country_id: CountryId,
    pub active_tool: ToolType,
    pub active_mode: EditorMode,
    pub display_mode: DisplayMode,
    pub brush_radius: u32,
    pub undo_manager: UndoManager,
    pub flag_cache: HashMap<CountryId, (u32, u32, Vec<u8>)>,
    pub component_grid: Vec<u16>,
    pub component_boxes: Vec<(CountryId, u32, u32, u32, u32)>,
    pub country_bounds: HashMap<CountryId, (u32, u32, u32, u32)>,
    pub components_dirty: bool,
    pub stats_dirty: bool,
}

impl WorldState {
    pub fn new_empty(name: impl Into<String>) -> Self {
        Self {
            name: name.into(),
            width: WORLD_WIDTH,
            height: WORLD_HEIGHT,
            terrain: TerrainGrid::new_empty(),
            political: PoliticalGrid::new_empty(),
            countries: Vec::new(),
            next_country_id: 1,
            active_country_id: 0,
            active_tool: ToolType::Brush,
            active_mode: EditorMode::Terrain,
            display_mode: DisplayMode::FlatColor,
            brush_radius: 16,
            undo_manager: UndoManager::new(30),
            flag_cache: HashMap::new(),
            component_grid: Vec::new(),
            component_boxes: Vec::new(),
            country_bounds: HashMap::new(),
            components_dirty: true,
            stats_dirty: true,
        }
    }

    pub fn new_with_terrain(name: impl Into<String>, terrain_data: Vec<u8>) -> Result<Self, &'static str> {
        let terrain = TerrainGrid::from_data(terrain_data)?;
        Ok(Self {
            name: name.into(),
            width: WORLD_WIDTH,
            height: WORLD_HEIGHT,
            terrain,
            political: PoliticalGrid::new_empty(),
            countries: Vec::new(),
            next_country_id: 1,
            active_country_id: 0,
            active_tool: ToolType::Brush,
            active_mode: EditorMode::Terrain,
            display_mode: DisplayMode::FlatColor,
            brush_radius: 16,
            undo_manager: UndoManager::new(30),
            flag_cache: HashMap::new(),
            component_grid: Vec::new(),
            component_boxes: Vec::new(),
            country_bounds: HashMap::new(),
            components_dirty: true,
            stats_dirty: true,
        })
    }

    pub fn recompute_country_pixel_counts(&mut self) {
        let mut counts: HashMap<CountryId, u64> = HashMap::new();
        for &cid in &self.political.data {
            if cid != 0 {
                *counts.entry(cid).or_insert(0) += 1;
            }
        }
        for c in &mut self.countries {
            c.pixel_count = counts.get(&c.id).copied().unwrap_or(0);
        }
        self.stats_dirty = false;
    }

    pub fn set_country_flag_rgba(&mut self, country_id: CountryId, width: u32, height: u32, data: Vec<u8>) {
        if width > 0 && height > 0 && data.len() >= (width * height * 4) as usize {
            self.flag_cache.insert(country_id, (width, height, data));
            self.components_dirty = true;
        }
    }

    pub fn set_country_flag(&mut self, country_id: CountryId, flag_path: &str) {
        if let Some(c) = self.get_country_mut(country_id) {
            c.flag_path = flag_path.to_string();
        }
        let p = Path::new(flag_path);
        if p.exists() {
            if let Ok(img) = image::open(p) {
                let resized = img.resize_exact(256, 170, image::imageops::FilterType::Nearest).to_rgba8();
                self.flag_cache.insert(country_id, (256, 170, resized.into_raw()));
                self.components_dirty = true;
            }
        }
    }

    pub fn set_display_mode(&mut self, mode: DisplayMode) {
        if self.display_mode != mode {
            self.display_mode = mode;
            if mode == DisplayMode::FlagPattern {
                self.components_dirty = true;
                self.update_components();
            }
        }
    }

    pub fn update_components(&mut self) {
        let scale = 16usize;
        let sw = (self.width as usize) / scale; // 512
        let sh = (self.height as usize) / scale; // 256
        let mut grid = vec![0u16; sw * sh];

        for sy in 0..sh {
            let py = (sy * scale + scale / 2).min(self.height as usize - 1);
            let row = py * (self.width as usize);
            for sx in 0..sw {
                let px = (sx * scale + scale / 2).min(self.width as usize - 1);
                grid[sy * sw + sx] = self.political.data[row + px];
            }
        }

        let mut visited = vec![false; sw * sh];
        let mut comp_grid = vec![0u16; sw * sh];
        let mut comp_boxes = vec![(0u16, 0u32, 0u32, 0u32, 0u32)]; // index 0 unused
        let mut country_bounds = HashMap::new();
        let mut next_comp_id = 1u16;

        for sy in 0..sh {
            for sx in 0..sw {
                let idx = sy * sw + sx;
                let cid = grid[idx];
                if cid == 0 || visited[idx] {
                    continue;
                }

                let mut queue = Vec::new();
                queue.push((sx, sy));
                visited[idx] = true;
                let comp_id = next_comp_id;
                next_comp_id = next_comp_id.saturating_add(1);
                comp_grid[idx] = comp_id;

                let mut min_cx = sx;
                let mut max_cx = sx;
                let mut min_cy = sy;
                let mut max_cy = sy;

                let mut head = 0;
                while head < queue.len() {
                    let (cx, cy) = queue[head];
                    head += 1;

                    if cx < min_cx { min_cx = cx; }
                    if cx > max_cx { max_cx = cx; }
                    if cy < min_cy { min_cy = cy; }
                    if cy > max_cy { max_cy = cy; }

                    let neighbors = [
                        (cx.wrapping_sub(1), cy),
                        (cx + 1, cy),
                        (cx, cy.wrapping_sub(1)),
                        (cx, cy + 1),
                    ];

                    for (nx, ny) in neighbors {
                        if nx < sw && ny < sh {
                            let nidx = ny * sw + nx;
                            if !visited[nidx] && grid[nidx] == cid {
                                visited[nidx] = true;
                                comp_grid[nidx] = comp_id;
                                queue.push((nx, ny));
                            }
                        }
                    }
                }

                let world_min_x = (min_cx as u32 * scale as u32).saturating_sub(scale as u32);
                let world_max_x = ((max_cx as u32 + 1) * scale as u32 + scale as u32).min(self.width - 1);
                let world_min_y = (min_cy as u32 * scale as u32).saturating_sub(scale as u32);
                let world_max_y = ((max_cy as u32 + 1) * scale as u32 + scale as u32).min(self.height - 1);

                comp_boxes.push((cid, world_min_x, world_min_y, world_max_x, world_max_y));

                let entry = country_bounds.entry(cid).or_insert((world_min_x, world_min_y, world_max_x, world_max_y));
                entry.0 = entry.0.min(world_min_x);
                entry.1 = entry.1.min(world_min_y);
                entry.2 = entry.2.max(world_max_x);
                entry.3 = entry.3.max(world_max_y);
            }
        }

        self.component_grid = comp_grid;
        self.component_boxes = comp_boxes;
        self.country_bounds = country_bounds;
        self.components_dirty = false;
    }

    pub fn ensure_components(&mut self) {
        if self.components_dirty || self.component_grid.is_empty() {
            self.update_components();
        }
    }

    pub fn create_country(&mut self, name: &str, color: ColorRgba) -> CountryId {
        let id = self.next_country_id;
        self.next_country_id += 1;
        self.countries.push(Country::new(id, name, color));
        self.active_country_id = id;
        self.stats_dirty = true;
        id
    }

    pub fn get_country(&self, id: CountryId) -> Option<&Country> {
        self.countries.iter().find(|c| c.id == id)
    }

    pub fn get_country_mut(&mut self, id: CountryId) -> Option<&mut Country> {
        self.countries.iter_mut().find(|c| c.id == id)
    }

    pub fn delete_country(&mut self, id: CountryId) -> Option<Rect> {
        let pos = self.countries.iter().position(|c| c.id == id)?;
        self.countries.remove(pos);
        self.flag_cache.remove(&id);
        if self.active_country_id == id {
            self.active_country_id = self.countries.first().map(|c| c.id).unwrap_or(0);
        }

        // Erase territory
        let mut min_x = self.width;
        let mut max_x = 0;
        let mut min_y = self.height;
        let mut max_y = 0;
        let mut found = false;

        for y in 0..self.height {
            let row = (y * self.width) as usize;
            for x in 0..self.width {
                if self.political.data[row + x as usize] == id {
                    self.political.data[row + x as usize] = 0;
                    if x < min_x { min_x = x; }
                    if x > max_x { max_x = x; }
                    if y < min_y { min_y = y; }
                    if y > max_y { max_y = y; }
                    found = true;
                }
            }
        }

        if found {
            self.components_dirty = true;
            self.stats_dirty = true;
            Some(Rect::new(min_x, min_y, max_x, max_y))
        } else {
            None
        }
    }

    pub fn pick_at(&self, x: u32, y: u32) -> CountryId {
        self.political.get(x, y)
    }

    pub fn paint_at(&mut self, x: u32, y: u32) -> Option<Rect> {
        if self.active_tool == ToolType::Hand {
            return None;
        }

        let radius = self.brush_radius;
        let rect = Rect::from_point_radius(x, y, radius);

        match self.active_mode {
            EditorMode::Terrain => {
                let val = match self.active_tool {
                    ToolType::Brush => 1, // Land
                    ToolType::Eraser => 0, // Water
                    _ => return None,
                };
                self.undo_manager.record_terrain_before(rect, &self.terrain);
                self.terrain.paint_circle(x, y, radius, val);
                Some(rect)
            }
            EditorMode::Political => {
                let target_country = match self.active_tool {
                    ToolType::Brush => self.active_country_id,
                    ToolType::Eraser => 0,
                    _ => return None,
                };
                self.undo_manager.record_political_before(rect, &self.political);
                let only_land = target_country != 0;
                self.political.paint_circle(x, y, radius, target_country, &self.terrain, only_land);
                self.components_dirty = true;
                self.stats_dirty = true;
                Some(rect)
            }
        }
    }

    pub fn fill_at(&mut self, x: u32, y: u32) -> Option<Rect> {
        if self.active_tool == ToolType::Hand {
            return None;
        }

        match self.active_mode {
            EditorMode::Terrain => {
                let target_val = 1; // Fill to land
                let current_val = self.terrain.get(x, y);
                if current_val == target_val {
                    return None;
                }
                if let Some(rect) = self.terrain.flood_fill(x, y, target_val) {
                    Some(rect)
                } else {
                    None
                }
            }
            EditorMode::Political => {
                let target_country = self.active_country_id;
                let current_country = self.political.get(x, y);
                if current_country == target_country {
                    return None;
                }
                if let Some(rect) = self.political.flood_fill(x, y, target_country, &self.terrain, true) {
                    self.components_dirty = true;
                    self.stats_dirty = true;
                    Some(rect)
                } else {
                    None
                }
            }
        }
    }

    pub fn undo(&mut self) -> Option<(Rect, EditorMode)> {
        self.components_dirty = true;
        self.stats_dirty = true;
        self.undo_manager.undo(&mut self.terrain, &mut self.political)
    }

    pub fn redo(&mut self) -> Option<(Rect, EditorMode)> {
        self.components_dirty = true;
        self.stats_dirty = true;
        self.undo_manager.redo(&mut self.terrain, &mut self.political)
    }

    /// Renders a rectangular region into a caller-provided 32-bit RGBA buffer with a custom byte stride per row.
    pub fn render_rect_to_rgba_strided(&mut self, rect: &Rect, buffer: &mut [u8], stride: usize) {
        let rw = rect.width() as usize;
        let rh = rect.height() as usize;
        let row_bytes = rw * 4;
        if rh == 0 || rw == 0 {
            return;
        }
        let total_required = (rh - 1) * stride + row_bytes;
        assert!(buffer.len() >= total_required, "Buffer too small for strided rect");

        let is_flag_mode = self.display_mode == DisplayMode::FlagPattern;
        if is_flag_mode {
            self.ensure_components();
        }

        // Fast O(1) LUT for nation colors to avoid O(N) linear scans per pixel
        let mut color_lut = vec![ColorRgba::new(180, 100, 180, 255); (self.next_country_id as usize) + 1];
        for c in &self.countries {
            if (c.id as usize) < color_lut.len() {
                color_lut[c.id as usize] = c.color;
            }
        }

        if !is_flag_mode {
            for (row_idx, y) in (rect.min_y..=rect.max_y).enumerate() {
                let row_offset = (y * self.width) as usize;
                let dest_offset = row_idx * stride;
                let dest_slice = &mut buffer[dest_offset..dest_offset + row_bytes];
                let mut buf_idx = 0;

                let min_x = rect.min_x as usize;
                let max_x = rect.max_x as usize;
                let terrain_slice = &self.terrain.data[row_offset + min_x..=row_offset + max_x];
                let political_slice = &self.political.data[row_offset + min_x..=row_offset + max_x];

                for (&is_land_byte, &cid) in terrain_slice.iter().zip(political_slice.iter()) {
                    let color = if cid > 0 {
                        color_lut.get(cid as usize).copied().unwrap_or(ColorRgba::new(180, 100, 180, 255))
                    } else if is_land_byte == 1 {
                        LAND_RGBA
                    } else {
                        WATER_RGBA
                    };

                    dest_slice[buf_idx] = color.r;
                    dest_slice[buf_idx + 1] = color.g;
                    dest_slice[buf_idx + 2] = color.b;
                    dest_slice[buf_idx + 3] = color.a;
                    buf_idx += 4;
                }
            }
        } else {
            for (row_idx, y) in (rect.min_y..=rect.max_y).enumerate() {
                let row_offset = (y * self.width) as usize;
                let dest_offset = row_idx * stride;
                let dest_slice = &mut buffer[dest_offset..dest_offset + row_bytes];
                let mut buf_idx = 0;

                for x in rect.min_x..=rect.max_x {
                    let idx = row_offset + x as usize;
                    let is_land = self.terrain.data[idx] == 1;
                    let country_id = self.political.data[idx];

                    let color = if country_id > 0 {
                        if let Some((flag_w, flag_h, flag_data)) = self.flag_cache.get(&country_id) {
                            let sx = (x >> 4) as usize;
                            let sy = (y >> 4) as usize;
                            let comp_id = if sx < 512 && sy < 256 {
                                self.component_grid.get(sy * 512 + sx).copied().unwrap_or(0) as usize
                            } else {
                                0
                            };

                            let (bx0, by0, bx1, by1) = if comp_id > 0 && comp_id < self.component_boxes.len() {
                                let (cid, x0, y0, x1, y1) = self.component_boxes[comp_id];
                                if cid == country_id {
                                    (x0, y0, x1, y1)
                                } else {
                                    self.country_bounds.get(&country_id).copied().unwrap_or((x, y, x, y))
                                }
                            } else {
                                self.country_bounds.get(&country_id).copied().unwrap_or((x, y, x, y))
                            };

                            let mid_y = (by0 + by1) as f32 * 0.5;
                            let center_lat = (std::f32::consts::PI * 0.5) - (mid_y * std::f32::consts::PI / (self.height as f32));
                            let cos_lat = center_lat.cos().abs().max(0.18);

                            let mut raw_bw = (bx1.saturating_sub(bx0) + 1) as f32;
                            if raw_bw > (self.width as f32) * 0.5 {
                                raw_bw = (self.width as f32) - raw_bw;
                            }
                            let eff_bw = (raw_bw * cos_lat).max(1.0);
                            let bh = (by1.saturating_sub(by0) + 1).max(1) as f32;
                            let aspect_flag = *flag_w as f32 / *flag_h as f32;
                            let aspect_box = eff_bw / bh;

                            let mid_x = (bx0 + bx1) as f32 * 0.5;
                            let mut dx = (x as f32) - mid_x;
                            if dx > (self.width as f32) * 0.5 {
                                dx -= self.width as f32;
                            } else if dx < -(self.width as f32) * 0.5 {
                                dx += self.width as f32;
                            }
                            let metric_dx = dx * cos_lat;

                            let (u, v) = if aspect_box > aspect_flag {
                                let u = (metric_dx / eff_bw) + 0.5;
                                let eff_h = eff_bw / aspect_flag;
                                let v = ((y as f32 - mid_y) / eff_h) + 0.5;
                                (u, v)
                            } else {
                                let eff_w = bh * aspect_flag;
                                let u = (metric_dx / eff_w) + 0.5;
                                let v = ((y as f32 - mid_y) / bh) + 0.5;
                                (u, v)
                            };

                            let u = u.clamp(0.0, 1.0);
                            let v = v.clamp(0.0, 1.0);

                            let fx = ((u * (*flag_w - 1) as f32) as usize).min(*flag_w as usize - 1);
                            let fy = ((v * (*flag_h - 1) as f32) as usize).min(*flag_h as usize - 1);
                            let fidx = (fy * *flag_w as usize + fx) * 4;

                            ColorRgba::new(
                                flag_data[fidx],
                                flag_data[fidx + 1],
                                flag_data[fidx + 2],
                                255,
                            )
                        } else {
                            color_lut.get(country_id as usize).copied().unwrap_or(ColorRgba::new(180, 100, 180, 255))
                        }
                    } else if is_land {
                        LAND_RGBA
                    } else {
                        WATER_RGBA
                    };

                    dest_slice[buf_idx] = color.r;
                    dest_slice[buf_idx + 1] = color.g;
                    dest_slice[buf_idx + 2] = color.b;
                    dest_slice[buf_idx + 3] = color.a;
                    buf_idx += 4;
                }
            }
        }
    }

    /// Renders a rectangular region into a caller-provided 32-bit RGBA buffer.
    pub fn render_rect_to_rgba(&mut self, rect: &Rect, buffer: &mut [u8]) {
        let rw = rect.width() as usize;
        self.render_rect_to_rgba_strided(rect, buffer, rw * 4);
    }

    pub fn save_to_file(&self, path: &Path) -> Result<(), Box<dyn std::error::Error>> {
        save::save_world_to_path(
            path,
            &self.name,
            &self.terrain.data,
            &self.political.data,
            &self.countries,
        )
    }

    pub fn load_from_file(path: &Path) -> Result<Self, Box<dyn std::error::Error>> {
        let bytes = std::fs::read(path)?;
        Self::load_from_bytes(&bytes)
    }

    pub fn load_from_bytes(bytes: &[u8]) -> Result<Self, Box<dyn std::error::Error>> {
        let (header, countries, terrain, political) = save::load_world_from_bytes(bytes)?;
        let mut world = Self {
            name: header.name,
            width: header.width,
            height: header.height,
            terrain: TerrainGrid::from_data(terrain)?,
            political: PoliticalGrid::from_data(political)?,
            next_country_id: countries.iter().map(|c| c.id).max().unwrap_or(0) + 1,
            active_country_id: countries.first().map(|c| c.id).unwrap_or(0),
            countries,
            active_tool: ToolType::Brush,
            active_mode: EditorMode::Terrain,
            display_mode: DisplayMode::FlatColor,
            brush_radius: 16,
            undo_manager: UndoManager::new(30),
            flag_cache: HashMap::new(),
            component_grid: Vec::new(),
            component_boxes: Vec::new(),
            country_bounds: HashMap::new(),
            components_dirty: true,
            stats_dirty: false,
        };
        for c in &world.countries {
            if !c.flag_path.is_empty() {
                let p = Path::new(&c.flag_path);
                if p.exists() {
                    if let Ok(img) = image::open(p) {
                        let resized = img.resize_exact(128, 64, image::imageops::FilterType::Nearest).to_rgba8();
                        world.flag_cache.insert(c.id, (128, 64, resized.into_raw()));
                    }
                }
            }
        }
        world.recompute_country_pixel_counts();
        Ok(world)
    }

    /// Computes centroid anchors for each connected territory component of each country
    pub fn compute_flag_anchors(&self) -> Vec<FlagAnchor> {
        let scale = 16usize;
        let sw = (self.width as usize) / scale;
        let sh = (self.height as usize) / scale;
        let mut grid = vec![0u16; sw * sh];

        for sy in 0..sh {
            let py = (sy * scale + scale / 2).min(self.height as usize - 1);
            let row = py * (self.width as usize);
            for sx in 0..sw {
                let px = (sx * scale + scale / 2).min(self.width as usize - 1);
                grid[sy * sw + sx] = self.political.data[row + px];
            }
        }

        let mut visited = vec![false; sw * sh];
        let mut anchors = Vec::new();

        for sy in 0..sh {
            for sx in 0..sw {
                let idx = sy * sw + sx;
                let cid = grid[idx];
                if cid == 0 || visited[idx] {
                    continue;
                }

                // Breadth-first search for this connected component
                let mut queue = Vec::new();
                queue.push((sx, sy));
                visited[idx] = true;

                let mut sum_x: u64 = 0;
                let mut sum_y: u64 = 0;
                let mut count: u32 = 0;
                let mut comp_points = Vec::new();

                let mut head = 0;
                while head < queue.len() {
                    let (cx, cy) = queue[head];
                    head += 1;

                    sum_x += cx as u64;
                    sum_y += cy as u64;
                    count += 1;
                    comp_points.push((cx, cy));

                    let neighbors = [
                        (cx.wrapping_sub(1), cy),
                        (cx + 1, cy),
                        (cx, cy.wrapping_sub(1)),
                        (cx, cy + 1),
                    ];

                    for (nx, ny) in neighbors {
                        if nx < sw && ny < sh {
                            let nidx = ny * sw + nx;
                            if !visited[nidx] && grid[nidx] == cid {
                                visited[nidx] = true;
                                queue.push((nx, ny));
                            }
                        }
                    }
                }

                // Minimum threshold: at least 3 sampled cells (approx ~750 pixels)
                if count >= 3 {
                    let mean_x = (sum_x / count as u64) as u32;
                    let mean_y = (sum_y / count as u64) as u32;

                    // Choose point inside component closest to mean
                    let mut best_pt = comp_points[0];
                    let mut best_dist = u64::MAX;
                    for &(px, py) in &comp_points {
                        let dx = (px as i64) - (mean_x as i64);
                        let dy = (py as i64) - (mean_y as i64);
                        let dist = (dx * dx + dy * dy) as u64;
                        if dist < best_dist {
                            best_dist = dist;
                            best_pt = (px, py);
                        }
                    }

                    anchors.push(FlagAnchor {
                        country_id: cid,
                        x: (best_pt.0 as u32 * scale as u32) + (scale as u32 / 2),
                        y: (best_pt.1 as u32 * scale as u32) + (scale as u32 / 2),
                        pixel_count: count * (scale * scale) as u32,
                    });
                }
            }
        }

        anchors
    }
}

#[derive(Debug, Clone, Copy)]
pub struct FlagAnchor {
    pub country_id: CountryId,
    pub x: u32,
    pub y: u32,
    pub pixel_count: u32,
}

