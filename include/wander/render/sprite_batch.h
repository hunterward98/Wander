#pragma once

#include "wander/core/types.h"
#include "wander/render/renderer.h"
#include <vector>

namespace wander {

// Batched sprite rendering — minimizes draw calls by grouping sprites
// with the same texture together
struct SpriteBatchItem {
    Texture* texture;
    Recti src;
    Rect dst;
    Color tint;
    f32 rotation;
    Vec2 origin;
    i32 layer;       // Sort key: lower = drawn first (background)
    bool flip_x;
    bool flip_y;
};

class SpriteBatch {
public:
    void begin();
    void draw(Texture* tex, Recti src, Rect dst, i32 layer = 0, Color tint = Color::white());
    void draw_ex(Texture* tex, Recti src, Rect dst, f32 rotation, Vec2 origin,
                 bool flip_x, bool flip_y, i32 layer = 0, Color tint = Color::white());
    void end(); // Sorts and flushes all queued draws

    u32 draw_call_count() const { return draw_calls_; }
    u32 sprite_count() const { return sprite_count_; }

private:
    std::vector<SpriteBatchItem> items_;
    u32 draw_calls_ = 0;
    u32 sprite_count_ = 0;
};

} // namespace wander
