#include "wander/render/tilemap.h"
#include "wander/core/log.h"
#include "wander/core/math.h"
#include <json.hpp>
#include <fstream>

using json = nlohmann::json;

namespace wander {

// Resolve which tileset a GID belongs to and get its source rect
static bool resolve_tile(const Tilemap& map, u32 gid, const TilesetRef** out_ts, Recti* out_src) {
    if (gid == 0) return false;

    // Find the tileset (tilesets are sorted by first_gid ascending)
    const TilesetRef* ts = nullptr;
    for (auto it = map.tilesets.rbegin(); it != map.tilesets.rend(); ++it) {
        if (gid >= it->first_gid) {
            ts = &(*it);
            break;
        }
    }
    if (!ts || !ts->texture.handle) return false;

    u32 local_id = gid - ts->first_gid;
    i32 col = static_cast<i32>(local_id) % ts->columns;
    i32 row = static_cast<i32>(local_id) / ts->columns;

    *out_ts = ts;
    *out_src = {col * ts->tile_width, row * ts->tile_height, ts->tile_width, ts->tile_height};
    return true;
}

bool tilemap_load(Tilemap& map, const char* json_path) {
    std::ifstream file(json_path);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open tilemap: %s", json_path);
        return false;
    }

    json data;
    try {
        data = json::parse(file);
    } catch (const json::exception& e) {
        LOG_ERROR("Failed to parse tilemap JSON: %s", e.what());
        return false;
    }

    map.width = data.value("width", 0);
    map.height = data.value("height", 0);
    map.tile_width = data.value("tilewidth", 0);
    map.tile_height = data.value("tileheight", 0);

    // Derive base path for relative asset paths
    std::string base_path(json_path);
    auto last_slash = base_path.find_last_of("/\\");
    if (last_slash != std::string::npos) {
        base_path = base_path.substr(0, last_slash + 1);
    } else {
        base_path = "";
    }

    // Load tilesets
    if (data.contains("tilesets")) {
        for (auto& ts_json : data["tilesets"]) {
            TilesetRef ts;
            ts.first_gid = ts_json.value("firstgid", 0u);
            ts.tile_count = ts_json.value("tilecount", 0u);
            ts.tile_width = ts_json.value("tilewidth", map.tile_width);
            ts.tile_height = ts_json.value("tileheight", map.tile_height);
            ts.columns = ts_json.value("columns", 1);

            if (ts_json.contains("image")) {
                ts.image_path = base_path + ts_json["image"].get<std::string>();
                ts.texture = texture_load(ts.image_path.c_str());
            }

            map.tilesets.push_back(std::move(ts));
        }
    }

    // Load layers
    if (data.contains("layers")) {
        for (auto& layer_json : data["layers"]) {
            std::string type = layer_json.value("type", "");

            if (type == "tilelayer") {
                TileLayer layer;
                layer.name = layer_json.value("name", "");
                layer.width = layer_json.value("width", map.width);
                layer.height = layer_json.value("height", map.height);
                layer.visible = layer_json.value("visible", true);
                layer.opacity = layer_json.value("opacity", 1.0f);

                if (layer_json.contains("data")) {
                    for (auto& gid : layer_json["data"]) {
                        layer.tiles.push_back(gid.get<u32>());
                    }
                }

                map.layers.push_back(std::move(layer));
            } else if (type == "objectgroup") {
                ObjectLayer ol;
                ol.name = layer_json.value("name", "");

                if (layer_json.contains("objects")) {
                    for (auto& obj_json : layer_json["objects"]) {
                        TileObject obj;
                        obj.id = obj_json.value("id", 0u);
                        obj.name = obj_json.value("name", "");
                        obj.type = obj_json.value("type", "");
                        obj.bounds.x = obj_json.value("x", 0.0f);
                        obj.bounds.y = obj_json.value("y", 0.0f);
                        obj.bounds.w = obj_json.value("width", 0.0f);
                        obj.bounds.h = obj_json.value("height", 0.0f);

                        if (obj_json.contains("polyline")) {
                            for (auto& pt : obj_json["polyline"]) {
                                obj.polyline.push_back({
                                    pt.value("x", 0.0f) + obj.bounds.x,
                                    pt.value("y", 0.0f) + obj.bounds.y
                                });
                            }
                        }

                        ol.objects.push_back(std::move(obj));
                    }
                }

                map.object_layers.push_back(std::move(ol));
            }
        }
    }

    LOG_INFO("Loaded tilemap: %s (%dx%d tiles, %zu layers, %zu tilesets)",
        json_path, map.width, map.height, map.layers.size(), map.tilesets.size());
    return true;
}

void tilemap_destroy(Tilemap& map) {
    for (auto& ts : map.tilesets) {
        texture_destroy(ts.texture);
    }
    map.layers.clear();
    map.object_layers.clear();
    map.tilesets.clear();
}

void tilemap_render(const Tilemap& map, const Camera2D& camera) {
    for (i32 i = 0; i < static_cast<i32>(map.layers.size()); i++) {
        if (map.layers[i].visible) {
            tilemap_render_layer(map, i, camera);
        }
    }
}

void tilemap_render_layer(const Tilemap& map, i32 layer_index, const Camera2D& camera) {
    if (layer_index < 0 || layer_index >= static_cast<i32>(map.layers.size())) return;
    const auto& layer = map.layers[layer_index];

    // Camera culling — determine visible tile range
    Rect view = camera.visible_rect();
    i32 start_x = clamp(static_cast<i32>(view.x) / map.tile_width, 0, layer.width);
    i32 start_y = clamp(static_cast<i32>(view.y) / map.tile_height, 0, layer.height);
    i32 end_x = clamp(static_cast<i32>(view.right()) / map.tile_width + 1, 0, layer.width);
    i32 end_y = clamp(static_cast<i32>(view.bottom()) / map.tile_height + 1, 0, layer.height);

    for (i32 y = start_y; y < end_y; y++) {
        for (i32 x = start_x; x < end_x; x++) {
            u32 gid = layer.tiles[y * layer.width + x];
            if (gid == 0) continue;

            const TilesetRef* ts;
            Recti src;
            if (!resolve_tile(map, gid, &ts, &src)) continue;

            Vec2 world_pos = {
                static_cast<f32>(x * map.tile_width),
                static_cast<f32>(y * map.tile_height)
            };
            Vec2 screen_pos = camera.world_to_screen(world_pos);

            Rect dst = {
                screen_pos.x,
                screen_pos.y,
                static_cast<f32>(map.tile_width) * camera.zoom,
                static_cast<f32>(map.tile_height) * camera.zoom
            };

            draw_texture_region(ts->texture, src, dst);
        }
    }
}

u32 tilemap_get_tile(const Tilemap& map, i32 layer_index, Vec2 world_pos) {
    if (layer_index < 0 || layer_index >= static_cast<i32>(map.layers.size())) return 0;
    const auto& layer = map.layers[layer_index];

    i32 tx = static_cast<i32>(world_pos.x) / map.tile_width;
    i32 ty = static_cast<i32>(world_pos.y) / map.tile_height;

    if (tx < 0 || tx >= layer.width || ty < 0 || ty >= layer.height) return 0;
    return layer.tiles[ty * layer.width + tx];
}

std::vector<const TileObject*> tilemap_find_objects(const Tilemap& map, const std::string& type) {
    std::vector<const TileObject*> result;
    for (auto& ol : map.object_layers) {
        for (auto& obj : ol.objects) {
            if (obj.type == type) {
                result.push_back(&obj);
            }
        }
    }
    return result;
}

} // namespace wander
