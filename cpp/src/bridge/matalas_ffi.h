#pragma once

#include <cstdint>
#include <cstddef>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    uint32_t min_x;
    uint32_t min_y;
    uint32_t max_x;
    uint32_t max_y;
} FfiRect;

typedef struct {
    uint16_t id;
    char name[64];
    uint8_t r;
    uint8_t g;
    uint8_t b;
    uint8_t a;
    uint64_t pixel_count;
} FfiCountryInfo;

typedef void* WorldStateHandle;

WorldStateHandle matalas_world_create_empty(const char* name);
WorldStateHandle matalas_world_create_from_preset(const char* name, const char* preset_png_or_bin);
void matalas_world_destroy(WorldStateHandle world);

bool matalas_world_paint_at(WorldStateHandle world, uint32_t x, uint32_t y, FfiRect* out_rect);
bool matalas_world_fill_at(WorldStateHandle world, uint32_t x, uint32_t y, FfiRect* out_rect);
uint16_t matalas_world_pick_at(WorldStateHandle world, uint32_t x, uint32_t y);

void matalas_world_set_tool(WorldStateHandle world, uint32_t tool_type);
void matalas_world_set_mode(WorldStateHandle world, uint32_t editor_mode);
void matalas_world_set_brush_radius(WorldStateHandle world, uint32_t radius);
void matalas_world_set_active_country(WorldStateHandle world, uint16_t country_id);

uint16_t matalas_world_create_country(WorldStateHandle world, const char* name, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
bool matalas_world_update_country(WorldStateHandle world, uint16_t id, const char* name, uint8_t r, uint8_t g, uint8_t b, uint8_t a);
bool matalas_world_delete_country(WorldStateHandle world, uint16_t id, FfiRect* out_dirty_rect);

bool matalas_world_undo(WorldStateHandle world, FfiRect* out_rect, uint32_t* out_mode);
bool matalas_world_redo(WorldStateHandle world, FfiRect* out_rect, uint32_t* out_mode);
bool matalas_world_can_undo(WorldStateHandle world);
bool matalas_world_can_redo(WorldStateHandle world);

bool matalas_world_render_rect(
    WorldStateHandle world,
    uint32_t min_x,
    uint32_t min_y,
    uint32_t max_x,
    uint32_t max_y,
    uint8_t* out_rgba_buffer,
    size_t buffer_len
);

size_t matalas_world_get_country_count(WorldStateHandle world);
bool matalas_world_get_country_info(WorldStateHandle world, size_t index, FfiCountryInfo* out_info);

bool matalas_world_save(WorldStateHandle world, const char* file_path);
WorldStateHandle matalas_world_load(const char* file_path);

#ifdef __cplusplus
}
#endif
