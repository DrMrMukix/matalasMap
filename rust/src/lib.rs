pub mod core;
pub mod ffi;
pub mod political;
pub mod save;
pub mod terrain;
pub mod undo;
pub mod world;

pub use world::WorldState;

#[cfg(test)]
mod tests {
    use super::*;
    use crate::core::types::{ColorRgba, EditorMode, Rect, ToolType};

    #[test]
    fn test_world_creation_and_terrain_painting() {
        let mut world = WorldState::new_empty("Test World");
        assert_eq!(world.terrain.get(100, 100), 0); // Initially water

        world.active_mode = EditorMode::Terrain;
        world.active_tool = ToolType::Brush;
        world.brush_radius = 5;

        let dirty = world.paint_at(100, 100).expect("Should paint dirty rect");
        assert!(dirty.min_x <= 100 && dirty.max_x >= 100);
        assert_eq!(world.terrain.get(100, 100), 1); // Now land!

        // Test Undo
        assert!(world.undo_manager.can_undo());
        let (_undo_rect, mode) = world.undo().expect("Undo should succeed");
        assert_eq!(mode, EditorMode::Terrain);
        assert_eq!(world.terrain.get(100, 100), 0); // Back to water!

        // Test Redo
        assert!(world.undo_manager.can_redo());
        let (_redo_rect, _) = world.redo().expect("Redo should succeed");
        assert_eq!(world.terrain.get(100, 100), 1); // Land again!
    }

    #[test]
    fn test_render_rect_strided() {
        let mut world = WorldState::new_empty("Render Test");
        // Paint land first
        world.brush_radius = 5;
        world.paint_at(10, 10);

        let cid = world.create_country("Testland", ColorRgba::new(200, 50, 50, 255));
        world.active_mode = EditorMode::Political;
        world.active_country_id = cid;
        world.brush_radius = 2;
        world.paint_at(10, 10);

        let rect = Rect::new(8, 8, 12, 12);
        let _rw = rect.width() as usize; // 5
        let rh = rect.height() as usize; // 5
        let stride = 64; // Stride larger than rw * 4 (20)
        let mut buffer = vec![0u8; rh * stride];

        world.render_rect_to_rgba_strided(&rect, &mut buffer, stride);

        // Center pixel (10, 10) corresponds to row_idx 2, col_idx 2 in rect (8..=12)
        let center_offset = 2 * stride + (2 * 4);
        assert_eq!(buffer[center_offset], 200);
        assert_eq!(buffer[center_offset + 1], 50);
        assert_eq!(buffer[center_offset + 2], 50);
        assert_eq!(buffer[center_offset + 3], 255);
    }

    #[test]
    fn test_political_country_creation_and_painting() {
        let mut world = WorldState::new_empty("Political Test");

        // First paint land
        world.brush_radius = 20;
        world.paint_at(200, 200);

        // Create country
        let cid = world.create_country("Atlantis", ColorRgba::new(255, 0, 0, 255));
        assert_eq!(cid, 1);
        assert_eq!(world.countries.len(), 1);

        world.active_mode = EditorMode::Political;
        world.active_tool = ToolType::Brush;
        world.active_country_id = cid;
        world.brush_radius = 5;

        world.paint_at(200, 200);
        assert_eq!(world.pick_at(200, 200), cid);

        // Undo political paint
        let (_, mode) = world.undo().expect("Undo political should succeed");
        assert_eq!(mode, EditorMode::Political);
        assert_eq!(world.pick_at(200, 200), 0);
    }

    #[test]
    fn test_save_and_load_roundtrip() {
        let mut world = WorldState::new_empty("SaveLoad Test");
        world.brush_radius = 10;
        world.paint_at(50, 50);
        let cid = world.create_country("Utopia", ColorRgba::new(0, 128, 255, 255));
        world.active_mode = EditorMode::Political;
        world.active_country_id = cid;
        world.paint_at(50, 50);

        let temp_dir = std::env::temp_dir();
        let save_path = temp_dir.join("matalas_test_save.matalas");

        world.save_to_file(&save_path).expect("Save should succeed");
        assert!(save_path.exists());

        let loaded = WorldState::load_from_file(&save_path).expect("Load should succeed");
        assert_eq!(loaded.name, "SaveLoad Test");
        assert_eq!(loaded.countries.len(), 1);
        assert_eq!(loaded.countries[0].name, "Utopia");
        assert_eq!(loaded.terrain.get(50, 50), 1);
        assert_eq!(loaded.political.get(50, 50), cid);

        let _ = std::fs::remove_file(save_path);
    }
}
