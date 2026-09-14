use crate::core::types::{ColorRgba, DisplayMode, EditorMode, Rect, ToolType, WORLD_HEIGHT, WORLD_WIDTH};
use crate::political::country::CountryId;
use crate::save;
use crate::world::WorldState;
use std::ffi::{CStr, CString};
use std::os::raw::{c_char, c_void};
use std::path::Path;

#[repr(C)]
pub struct FfiRect {
    pub min_x: u32,
    pub min_y: u32,
    pub max_x: u32,
    pub max_y: u32,
}

impl From<Rect> for FfiRect {
    fn from(r: Rect) -> Self {
        Self {
            min_x: r.min_x,
            min_y: r.min_y,
            max_x: r.max_x,
            max_y: r.max_y,
        }
    }
}

impl From<FfiRect> for Rect {
    fn from(r: FfiRect) -> Self {
        Rect::new(r.min_x, r.min_y, r.max_x, r.max_y)
    }
}

#[repr(C)]
pub struct FfiCountryInfo {
    pub id: u16,
    pub name: [c_char; 64],
    pub r: u8,
    pub g: u8,
    pub b: u8,
    pub a: u8,
    pub pixel_count: u64,
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_create_empty(name: *const c_char) -> *mut WorldState {
    let name_str = if !name.is_null() {
        CStr::from_ptr(name).to_str().unwrap_or("Nuevo Mundo")
    } else {
        "Nuevo Mundo"
    };

    let world = Box::new(WorldState::new_empty(name_str));
    Box::into_raw(world)
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_create_from_preset(
    name: *const c_char,
    preset_png_or_bin: *const c_char,
) -> *mut WorldState {
    let name_str = if !name.is_null() {
        CStr::from_ptr(name).to_str().unwrap_or("Tierra")
    } else {
        "Tierra"
    };

    let path_str = if !preset_png_or_bin.is_null() {
        CStr::from_ptr(preset_png_or_bin).to_str().unwrap_or("")
    } else {
        ""
    };

    let p = Path::new(path_str);
    let mut terrain_data = vec![0u8; (WORLD_WIDTH * WORLD_HEIGHT) as usize];

    if p.exists() {
        if path_str.ends_with(".png") {
            if let Ok(img) = image::open(p) {
                let rgba = img.to_rgba8();
                for (i, pixel) in rgba.pixels().enumerate() {
                    if i < terrain_data.len() {
                        // Land is #4E9A51 (Green component > 100 and Blue component < 100)
                        let [r, g, b, _] = pixel.0;
                        if g > 120 && b < 100 {
                            terrain_data[i] = 1;
                        } else {
                            terrain_data[i] = 0;
                        }
                    }
                }
            }
        } else if path_str.ends_with(".bin.gz") {
            if let Ok(gz_bytes) = std::fs::read(p) {
                if let Ok(decomp) = save::decompress_bytes(&gz_bytes) {
                    if decomp.len() == terrain_data.len() {
                        terrain_data = decomp;
                    }
                }
            }
        }
    }

    match WorldState::new_with_terrain(name_str, terrain_data) {
        Ok(world) => Box::into_raw(Box::new(world)),
        Err(_) => Box::into_raw(Box::new(WorldState::new_empty(name_str))),
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_destroy(world: *mut WorldState) {
    if !world.is_null() {
        drop(Box::from_raw(world));
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_paint_at(
    world: *mut WorldState,
    x: u32,
    y: u32,
    out_rect: *mut FfiRect,
) -> bool {
    if world.is_null() {
        return false;
    }
    let w = &mut *world;
    if let Some(rect) = w.paint_at(x, y) {
        if !out_rect.is_null() {
            *out_rect = rect.into();
        }
        true
    } else {
        false
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_fill_at(
    world: *mut WorldState,
    x: u32,
    y: u32,
    out_rect: *mut FfiRect,
) -> bool {
    if world.is_null() {
        return false;
    }
    let w = &mut *world;
    if let Some(rect) = w.fill_at(x, y) {
        if !out_rect.is_null() {
            *out_rect = rect.into();
        }
        true
    } else {
        false
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_pick_at(world: *mut WorldState, x: u32, y: u32) -> u16 {
    if world.is_null() {
        return 0;
    }
    (*world).pick_at(x, y)
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_set_tool(world: *mut WorldState, tool_type: u32) {
    if !world.is_null() {
        (*world).active_tool = ToolType::from(tool_type);
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_set_mode(world: *mut WorldState, editor_mode: u32) {
    if !world.is_null() {
        (*world).active_mode = EditorMode::from(editor_mode);
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_set_brush_radius(world: *mut WorldState, radius: u32) {
    if !world.is_null() {
        (*world).brush_radius = radius.max(1);
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_set_active_country(world: *mut WorldState, country_id: u16) {
    if !world.is_null() {
        (*world).active_country_id = country_id;
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_create_country(
    world: *mut WorldState,
    name: *const c_char,
    r: u8,
    g: u8,
    b: u8,
    a: u8,
) -> u16 {
    if world.is_null() {
        return 0;
    }
    let name_str = if !name.is_null() {
        CStr::from_ptr(name).to_str().unwrap_or("Nuevo País")
    } else {
        "Nuevo País"
    };

    (*world).create_country(name_str, ColorRgba::new(r, g, b, a))
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_update_country(
    world: *mut WorldState,
    id: u16,
    name: *const c_char,
    r: u8,
    g: u8,
    b: u8,
    a: u8,
) -> bool {
    if world.is_null() {
        return false;
    }
    if let Some(c) = (*world).get_country_mut(id) {
        if !name.is_null() {
            if let Ok(s) = CStr::from_ptr(name).to_str() {
                c.name = s.to_string();
            }
        }
        c.color = ColorRgba::new(r, g, b, a);
        true
    } else {
        false
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_delete_country(
    world: *mut WorldState,
    id: u16,
    out_dirty_rect: *mut FfiRect,
) -> bool {
    if world.is_null() {
        return false;
    }
    if let Some(rect) = (*world).delete_country(id) {
        if !out_dirty_rect.is_null() {
            *out_dirty_rect = rect.into();
        }
        true
    } else {
        false
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_undo(
    world: *mut WorldState,
    out_rect: *mut FfiRect,
    out_mode: *mut u32,
) -> bool {
    if world.is_null() {
        return false;
    }
    if let Some((rect, mode)) = (*world).undo() {
        if !out_rect.is_null() {
            *out_rect = rect.into();
        }
        if !out_mode.is_null() {
            *out_mode = mode as u32;
        }
        true
    } else {
        false
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_redo(
    world: *mut WorldState,
    out_rect: *mut FfiRect,
    out_mode: *mut u32,
) -> bool {
    if world.is_null() {
        return false;
    }
    if let Some((rect, mode)) = (*world).redo() {
        if !out_rect.is_null() {
            *out_rect = rect.into();
        }
        if !out_mode.is_null() {
            *out_mode = mode as u32;
        }
        true
    } else {
        false
    }
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_can_undo(world: *mut WorldState) -> bool {
    if world.is_null() {
        return false;
    }
    (*world).undo_manager.can_undo()
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_can_redo(world: *mut WorldState) -> bool {
    if world.is_null() {
        return false;
    }
    (*world).undo_manager.can_redo()
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_render_rect(
    world: *mut WorldState,
    min_x: u32,
    min_y: u32,
    max_x: u32,
    max_y: u32,
    out_rgba_buffer: *mut u8,
    buffer_len: usize,
) -> bool {
    if world.is_null() || out_rgba_buffer.is_null() {
        return false;
    }

    let rect = Rect::new(min_x, min_y, max_x, max_y);
    let needed = (rect.width() * rect.height() * 4) as usize;
    if buffer_len < needed {
        return false;
    }

    let slice = std::slice::from_raw_parts_mut(out_rgba_buffer, needed);
    (*world).render_rect_to_rgba(&rect, slice);
    true
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_get_country_count(world: *mut WorldState) -> usize {
    if world.is_null() {
        return 0;
    }
    (*world).countries.len()
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_get_country_info(
    world: *mut WorldState,
    index: usize,
    out_info: *mut FfiCountryInfo,
) -> bool {
    if world.is_null() || out_info.is_null() {
        return false;
    }
    let countries = &(*world).countries;
    if index >= countries.len() {
        return false;
    }

    let c = &countries[index];
    let mut info = FfiCountryInfo {
        id: c.id,
        name: [0; 64],
        r: c.color.r,
        g: c.color.g,
        b: c.color.b,
        a: c.color.a,
        pixel_count: c.pixel_count,
    };

    let bytes = c.name.as_bytes();
    let len = bytes.len().min(63);
    for i in 0..len {
        info.name[i] = bytes[i] as c_char;
    }

    *out_info = info;
    true
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_save(world: *mut WorldState, file_path: *const c_char) -> bool {
    if world.is_null() || file_path.is_null() {
        return false;
    }
    let path_str = match CStr::from_ptr(file_path).to_str() {
        Ok(s) => s,
        Err(_) => return false,
    };

    (*world).save_to_file(Path::new(path_str)).is_ok()
}

#[no_mangle]
pub unsafe extern "C" fn matalas_world_load(file_path: *const c_char) -> *mut WorldState {
    if file_path.is_null() {
        return std::ptr::null_mut();
    }
    let path_str = match CStr::from_ptr(file_path).to_str() {
        Ok(s) => s,
        Err(_) => return std::ptr::null_mut(),
    };

    match WorldState::load_from_file(Path::new(path_str)) {
        Ok(w) => Box::into_raw(Box::new(w)),
        Err(_) => std::ptr::null_mut(),
    }
}
