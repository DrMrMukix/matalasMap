use serde::{Deserialize, Serialize};

pub const WORLD_WIDTH: u32 = 8192;
pub const WORLD_HEIGHT: u32 = 4096;
pub const TOTAL_PIXELS: usize = (WORLD_WIDTH * WORLD_HEIGHT) as usize;

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct Rect {
    pub min_x: u32,
    pub min_y: u32,
    pub max_x: u32,
    pub max_y: u32,
}

impl Rect {
    pub fn new(min_x: u32, min_y: u32, max_x: u32, max_y: u32) -> Self {
        Self {
            min_x: min_x.min(WORLD_WIDTH.saturating_sub(1)),
            min_y: min_y.min(WORLD_HEIGHT.saturating_sub(1)),
            max_x: max_x.min(WORLD_WIDTH.saturating_sub(1)),
            max_y: max_y.min(WORLD_HEIGHT.saturating_sub(1)),
        }
    }

    pub fn from_point_radius(cx: u32, cy: u32, radius: u32) -> Self {
        Self::new(
            cx.saturating_sub(radius),
            cy.saturating_sub(radius),
            (cx + radius).min(WORLD_WIDTH - 1),
            (cy + radius).min(WORLD_HEIGHT - 1),
        )
    }

    pub fn union(&self, other: &Rect) -> Rect {
        Rect {
            min_x: self.min_x.min(other.min_x),
            min_y: self.min_y.min(other.min_y),
            max_x: self.max_x.max(other.max_x),
            max_y: self.max_y.max(other.max_y),
        }
    }

    pub fn width(&self) -> u32 {
        if self.max_x >= self.min_x {
            self.max_x - self.min_x + 1
        } else {
            0
        }
    }

    pub fn height(&self) -> u32 {
        if self.max_y >= self.min_y {
            self.max_y - self.min_y + 1
        } else {
            0
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq, Serialize, Deserialize)]
pub struct ColorRgba {
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
}

impl ColorRgba {
    pub const fn new(r: u8, g: u8, b: u8, a: u8) -> Self {
        Self { r, g, b, a }
    }

    pub const fn to_u32(&self) -> u32 {
        ((self.a as u32) << 24) | ((self.r as u32) << 16) | ((self.g as u32) << 8) | (self.b as u32)
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum ToolType {
    Brush = 0,
    Eraser = 1,
    Fill = 2,
    Picker = 3,
    Hand = 4,
}

impl From<u32> for ToolType {
    fn from(val: u32) -> Self {
        match val {
            0 => ToolType::Brush,
            1 => ToolType::Eraser,
            2 => ToolType::Fill,
            3 => ToolType::Picker,
            _ => ToolType::Hand,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum EditorMode {
    Terrain = 0,
    Political = 1,
}

impl From<u32> for EditorMode {
    fn from(val: u32) -> Self {
        match val {
            1 => EditorMode::Political,
            _ => EditorMode::Terrain,
        }
    }
}

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum DisplayMode {
    FlatColor = 0,
    FlagPattern = 1,
}

impl From<u32> for DisplayMode {
    fn from(val: u32) -> Self {
        match val {
            1 => DisplayMode::FlagPattern,
            _ => DisplayMode::FlatColor,
        }
    }
}
