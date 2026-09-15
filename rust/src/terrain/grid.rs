use crate::core::types::{Rect, TOTAL_PIXELS, WORLD_HEIGHT, WORLD_WIDTH};

#[derive(Clone)]
pub struct TerrainGrid {
    pub width: u32,
    pub height: u32,
    pub data: Vec<u8>,
}

impl TerrainGrid {
    pub fn new_empty() -> Self {
        Self {
            width: WORLD_WIDTH,
            height: WORLD_HEIGHT,
            data: vec![0u8; TOTAL_PIXELS],
        }
    }

    pub fn from_data(data: Vec<u8>) -> Result<Self, &'static str> {
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
    pub fn get(&self, x: u32, y: u32) -> u8 {
        if x < self.width && y < self.height {
            self.data[(y * self.width + x) as usize]
        } else {
            0
        }
    }

    #[inline(always)]
    pub fn set(&mut self, x: u32, y: u32, val: u8) {
        if x < self.width && y < self.height {
            self.data[(y * self.width + x) as usize] = val;
        }
    }

    pub fn paint_circle(&mut self, cx: u32, cy: u32, radius: u32, val: u8) -> Rect {
        let r_i32 = radius as i32;
        let cx_i32 = cx as i32;
        let cy_i32 = cy as i32;
        let w_i32 = self.width as i32;
        let h_i32 = self.height as i32;

        let min_y = (cy_i32 - r_i32).max(0) as u32;
        let max_y = (cy_i32 + r_i32).min(h_i32 - 1) as u32;

        let r2 = r_i32 * r_i32;
        let wraps = (cx_i32 - r_i32 < 0) || (cx_i32 + r_i32 >= w_i32);

        if !wraps {
            let min_x = (cx_i32 - r_i32).max(0) as u32;
            let max_x = (cx_i32 + r_i32).min(w_i32 - 1) as u32;

            for y in min_y..=max_y {
                let dy = y as i32 - cy_i32;
                let dy2 = dy * dy;
                let row_offset = (y * self.width) as usize;

                let max_dx = ((r2 - dy2) as f64).sqrt() as i32;
                let x0 = (cx_i32 - max_dx) as usize;
                let x1 = (cx_i32 + max_dx) as usize;
                self.data[row_offset + x0..=row_offset + x1].fill(val);
            }

            Rect::new(min_x, min_y, max_x, max_y)
        } else {
            for y in min_y..=max_y {
                let dy = y as i32 - cy_i32;
                let dy2 = dy * dy;
                let row_offset = (y * self.width) as usize;

                let max_dx = ((r2 - dy2) as f64).sqrt() as i32;
                for dx in -max_dx..=max_dx {
                    let x = (cx_i32 + dx).rem_euclid(w_i32) as usize;
                    self.data[row_offset + x] = val;
                }
            }

            Rect::new(0, min_y, self.width - 1, max_y)
        }
    }

    pub fn flood_fill(&mut self, start_x: u32, start_y: u32, target_val: u8) -> Option<Rect> {
        if start_x >= self.width || start_y >= self.height {
            return None;
        }

        let origin_val = self.get(start_x, start_y);
        if origin_val == target_val {
            return None;
        }

        let mut stack = Vec::with_capacity(2048);
        stack.push((start_x, start_y));

        let mut min_x = start_x;
        let mut max_x = start_x;
        let mut min_y = start_y;
        let mut max_y = start_y;

        let w = self.width as usize;
        let h = self.height as usize;

        while let Some((x, y)) = stack.pop() {
            let row_offset = (y as usize) * w;
            let current_idx = row_offset + (x as usize);
            if self.data[current_idx] != origin_val {
                continue;
            }

            // Expand span to the left
            let mut lx = x as usize;
            while lx > 0 && self.data[row_offset + lx - 1] == origin_val {
                lx -= 1;
            }

            // Expand span to the right
            let mut rx = x as usize;
            while rx + 1 < w && self.data[row_offset + rx + 1] == origin_val {
                rx += 1;
            }

            // Vectorized span fill
            self.data[row_offset + lx..=row_offset + rx].fill(target_val);

            let lx_u32 = lx as u32;
            let rx_u32 = rx as u32;
            if lx_u32 < min_x { min_x = lx_u32; }
            if rx_u32 > max_x { max_x = rx_u32; }
            if y < min_y { min_y = y; }
            if y > max_y { max_y = y; }

            // Scan adjacent rows for seeds
            let mut scan_row = |adj_y: usize| {
                let adj_offset = adj_y * w;
                let mut in_span = false;
                for cx in lx..=rx {
                    if self.data[adj_offset + cx] == origin_val {
                        if !in_span {
                            stack.push((cx as u32, adj_y as u32));
                            in_span = true;
                        }
                    } else {
                        in_span = false;
                    }
                }
            };

            if (y as usize) > 0 {
                scan_row((y as usize) - 1);
            }
            if (y as usize) + 1 < h {
                scan_row((y as usize) + 1);
            }
        }

        Some(Rect::new(min_x, min_y, max_x, max_y))
    }

    pub fn extract_sub_rect(&self, rect: &Rect) -> Vec<u8> {
        let w = rect.width() as usize;
        let h = rect.height() as usize;
        let mut buffer = Vec::with_capacity(w * h);

        for y in rect.min_y..=rect.max_y {
            let row_start = (y * self.width + rect.min_x) as usize;
            buffer.extend_from_slice(&self.data[row_start..row_start + w]);
        }

        buffer
    }

    pub fn restore_sub_rect(&mut self, rect: &Rect, buffer: &[u8]) {
        let w = rect.width() as usize;
        let mut offset = 0;

        for y in rect.min_y..=rect.max_y {
            let row_start = (y * self.width + rect.min_x) as usize;
            self.data[row_start..row_start + w].copy_from_slice(&buffer[offset..offset + w]);
            offset += w;
        }
    }
}
