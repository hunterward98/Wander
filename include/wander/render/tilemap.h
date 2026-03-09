#pragma once

#include "wander/core/types.h"
#include "wander/render/renderer.h"
#include "wander/render/camera.h"
#include <vector>
#include <string>

namespace wander {

// A single tile layer
struct TileLayer {
    std::string name;
    i32 width = 0;           // Width in tiles
    i32 height = 0;          // Height in tiles
    std::vector<u32> tiles;  // Tile GIDs (0 = empty)
    bool visible = true;
    f32 opacity = 1.0f;
};

// Object in an object layer (triggers, spawn points, waypoints)
struct TileObject {
    u32 id = 0;
    std::string name;
    std::string type;
    Rect bounds;
    std::vector<Vec2> polyline;  // For path-based objects (tower defense paths)
};

// An object layer
struct ObjectLayer {
    std::string name;
    std::vector<TileObject> objects;
};

// Tileset reference
struct TilesetRef {
    u32 first_gid = 0;
    u32 tile_count = 0;
    i32 tile_width = 0;
    i32 tile_height = 0;
    i32 columns = 0;
    Texture texture;
    std::string image_path;
};

// Complete tilemap loaded from Tiled JSON
struct Tilemap {
    i32 width = 0;            // Map width in tiles
    i32 height = 0;           // Map height in tiles
    i32 tile_width = 0;       // Tile width in pixels
    i32 tile_height = 0;      // Tile height in pixels
    std::vector<TileLayer> layers;
    std::vector<ObjectLayer> object_layers;
    std::vector<TilesetRef> tilesets;
};

// Load a Tiled JSON map
bool tilemap_load(Tilemap& map, const char* json_path);

// Free tilemap textures
void tilemap_destroy(Tilemap& map);

// Render visible tile layers with camera culling
void tilemap_render(const Tilemap& map, const Camera2D& camera);

// Render a specific layer by index
void tilemap_render_layer(const Tilemap& map, i32 layer_index, const Camera2D& camera);

// Get tile GID at a world position in a given layer
u32 tilemap_get_tile(const Tilemap& map, i32 layer_index, Vec2 world_pos);

// Find objects by type across all object layers
std::vector<const TileObject*> tilemap_find_objects(const Tilemap& map, const std::string& type);

} // namespace wander
