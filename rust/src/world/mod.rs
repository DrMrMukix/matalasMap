use crate::core::types::{
    ColorRgba, DisplayMode, EditorMode, Rect, ToolType, TOTAL_PIXELS, WORLD_HEIGHT, WORLD_WIDTH,
};
use crate::political::country::{Country, CountryId};
use crate::political::grid::PoliticalGrid;
use crate::save;
use crate::terrain::grid::TerrainGrid;
use crate::undo::UndoManager;
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
        })
    }

    pub fn create_country(&mut self, name: &str, color: ColorRgba) -> CountryId {
        let id = self.next_country_id;
        self.next_country_id += 1;
        self.countries.push(Country::new(id, name, color));
        self.active_country_id = id;
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
            Some(Rect::new(min_x, min_y, max_x, max_y))
        } else {
            None
        }
    }

    pub fn pick_at(&self, x: u32, y: u32) -> CountryId {
        self.political.get(x, y)
    }

    pub fn paint_at(&mut self, x: u32, y: u32) -> Option<Rect> {
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
                // In political mode: painting can only happen on land unless erasing
                let only_land = target_country != 0;
                self.political.paint_circle(x, y, radius, target_country, &self.terrain, only_land);
                Some(rect)
            }
        }
    }

    pub fn fill_at(&mut self, x: u32, y: u32) -> Option<Rect> {
        match self.active_mode {
            EditorMode::Terrain => {
                let target_val = 1; // Fill to land
                // Before flood fill, we don't know the exact bounding box yet, but we can snapshot or do fill
                // For undo on flood fill, a snapshot of the resulting bounding box is recorded
                let current_val = self.terrain.get(x, y);
                if current_val == target_val {
                    return None;
                }
                // Perform fill
                if let Some(rect) = self.terrain.flood_fill(x, y, target_val) {
                    // Record rect
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
                    Some(rect)
                } else {
                    None
                }
            }
        }
    }

    pub fn undo(&mut self) -> Option<(Rect, EditorMode)> {
        self.undo_manager.undo(&mut self.terrain, &mut self.political)
    }

    pub fn redo(&mut self) -> Option<(Rect, EditorMode)> {
        self.undo_manager.redo(&mut self.terrain, &mut self.political)
    }

    /// Renders a rectangular region into a caller-provided 32-bit RGBA buffer.
    /// Buffer size must be rect.width() * rect.height() * 4 bytes.
    pub fn render_rect_to_rgba(&self, rect: &Rect, buffer: &mut [u8]) {
        let rw = rect.width() as usize;
        let rh = rect.height() as usize;
        assert!(buffer.len() >= rw * rh * 4, "Buffer too small for rect");

        let mut buf_idx = 0;

        for y in rect.min_y..=rect.max_y {
            let row_offset = (y * self.width) as usize;
            for x in rect.min_x..=rect.max_x {
                let idx = row_offset + x as usize;
                let is_land = self.terrain.data[idx] == 1;
                let country_id = self.political.data[idx];

                let color = if country_id > 0 {
                    if let Some(country) = self.get_country(country_id) {
                        country.color
                    } else {
                        ColorRgba::new(180, 100, 180, 255)
                    }
                } else if is_land {
                    LAND_RGBA
                } else {
                    WATER_RGBA
                };

                buffer[buf_idx] = color.r;
                buffer[buf_idx + 1] = color.g;
                buffer[buf_idx + 2] = color.b;
                buffer[buf_idx + 3] = color.a;
                buf_idx += 4;
            }
        }
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
        let (header, countries, terrain, political) = save::load_world_from_path(path)?;
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
        };
        Ok(world)
    }
}
