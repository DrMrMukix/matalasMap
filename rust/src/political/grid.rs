use crate::core::types::{Rect, TOTAL_PIXELS, WORLD_HEIGHT, WORLD_WIDTH};
use crate::political::country::CountryId;
use crate::terrain::grid::TerrainGrid;
use std::collections::VecDeque;

#[derive(Clone)]
pub struct PoliticalGrid {
    pub width: u32,
    pub height: u32,
    pub data: Vec<CountryId>,
}

impl PoliticalGrid {
    pub fn new_empty() -> Self {
        Self {
            width: WORLD_WIDTH,
            height: WORLD_HEIGHT,
            data: vec![0u16; TOTAL_PIXELS],
        }
    }

    pub fn from_data(data: Vec<CountryId>) -> Result<Self, &'static str> {
        if data.len() != TOTAL_PIXELS {
            return Err("Data size does not match world dimensions");
        }
        Ok(Self {
            width: WORLD_WIDTH,
            height: WORLD_HEIGHT,
            data,
        })
    }

    #[inline(always)]
    pub fn get(&self, x: u32, y: u32) -> CountryId {
        if x < self.width && y < self.height {
            self.data[(y * self.width + x) as usize]
        } else {
            0
        }
    }

    #[inline(always)]
    pub fn set(&mut self, x: u32, y: u32, val: CountryId) {
        if x < self.width && y < self.height {
            self.data[(y * self.width + x) as usize] = val;
        }
    }

    pub fn paint_circle(
        &mut self,
        cx: u32,
        cy: u32,
        radius: u32,
        country_id: CountryId,
        terrain: &TerrainGrid,
        only_land: bool,
    ) -> Rect {
        let r_i32 = radius as i32;
        let cx_i32 = cx as i32;
        let cy_i32 = cy as i32;
        let w_i32 = self.width as i32;
        let h_i32 = self.height as i32;

        let min_x = (cx_i32 - r_i32).max(0) as u32;
        let max_x = (cx_i32 + r_i32).min(w_i32 - 1) as u32;
        let min_y = (cy_i32 - r_i32).max(0) as u32;
        let max_y = (cy_i32 + r_i32).min(h_i32 - 1) as u32;

        let r2 = r_i32 * r_i32;

        for y in min_y..=max_y {
            let dy = y as i32 - cy_i32;
            let dy2 = dy * dy;
            let row_offset = (y * self.width) as usize;

            for x in min_x..=max_x {
                let dx = x as i32 - cx_i32;
                if dx * dx + dy2 <= r2 {
                    let idx = row_offset + x as usize;
                    if !only_land || terrain.data[idx] == 1 {
                        self.data[idx] = country_id;
                    }
                }
            }
        }

        Rect::new(min_x, min_y, max_x, max_y)
    }

    pub fn flood_fill(
        &mut self,
        start_x: u32,
        start_y: u32,
        target_country: CountryId,
        terrain: &TerrainGrid,
        restrict_to_same_terrain: bool,
    ) -> Option<Rect> {
        if start_x >= self.width || start_y >= self.height {
            return None;
        }

        let origin_country = self.get(start_x, start_y);
        if origin_country == target_country {
            return None;
        }

        let origin_terrain = terrain.get(start_x, start_y);
        // If we are painting political territory and user clicks on water, only allow if not restricted
        if restrict_to_same_terrain && origin_terrain == 0 && target_country != 0 {
            // Usually political borders apply on land, but user can also erase or claim waters
        }

        let mut queue = VecDeque::with_capacity(4096);
        queue.push_back((start_x, start_y));
        self.set(start_x, start_y, target_country);

        let mut min_x = start_x;
        let mut max_x = start_x;
        let mut min_y = start_y;
        let mut max_y = start_y;

        let mut count = 0usize;
        let max_fill = TOTAL_PIXELS;

        while let Some((x, y)) = queue.pop_front() {
            count += 1;
            if count > max_fill {
                break;
            }

            if x < min_x { min_x = x; }
            if x > max_x { max_x = x; }
            if y < min_y { min_y = y; }
            if y > max_y { max_y = y; }

            let neighbors = [
                (x.wrapping_sub(1), y, x > 0),
                (x + 1, y, x + 1 < self.width),
                (x, y.wrapping_sub(1), y > 0),
                (x, y + 1, y + 1 < self.height),
            ];

            for (nx, ny, valid) in neighbors {
                if valid {
                    let matches_country = self.get(nx, ny) == origin_country;
                    let matches_terrain = !restrict_to_same_terrain || terrain.get(nx, ny) == origin_terrain;

                    if matches_country && matches_terrain {
                        self.set(nx, ny, target_country);
                        queue.push_back((nx, ny));
                    }
                }
            }
        }

        Some(Rect::new(min_x, min_y, max_x, max_y))
    }

    pub fn extract_sub_rect(&self, rect: &Rect) -> Vec<CountryId> {
        let w = rect.width() as usize;
        let h = rect.height() as usize;
        let mut buffer = Vec::with_capacity(w * h);

        for y in rect.min_y..=rect.max_y {
            let row_start = (y * self.width + rect.min_x) as usize;
            buffer.extend_from_slice(&self.data[row_start..row_start + w]);
        }

        buffer
    }

    pub fn restore_sub_rect(&mut self, rect: &Rect, buffer: &[CountryId]) {
        let w = rect.width() as usize;
        let mut offset = 0;

        for y in rect.min_y..=rect.max_y {
            let row_start = (y * self.width + rect.min_x) as usize;
            self.data[row_start..row_start + w].copy_from_slice(&buffer[offset..offset + w]);
            offset += w;
        }
    }
}
