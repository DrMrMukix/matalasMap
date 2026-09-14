use crate::core::types::ColorRgba;
use serde::{Deserialize, Serialize};

pub type CountryId = u16;

#[derive(Debug, Clone, Serialize, Deserialize)]
pub struct Country {
    pub id: CountryId,
    pub name: String,
    pub color: ColorRgba,
    pub flag_path: String,
    pub pixel_count: u64,
}

impl Country {
    pub fn new(id: CountryId, name: impl Into<String>, color: ColorRgba) -> Self {
        Self {
            id,
            name: name.into(),
            color,
            flag_path: String::new(),
            pixel_count: 0,
        }
    }
}
