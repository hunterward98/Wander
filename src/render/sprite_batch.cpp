#include "wander/render/sprite_batch.h"
#include <algorithm>

namespace wander {

void SpriteBatch::begin() {
    items_.clear();
    draw_calls_ = 0;
    sprite_count_ = 0;
}

void SpriteBatch::draw(Texture* tex, Recti src, Rect dst, i32 layer, Color tint) {
    items_.push_back({tex, src, dst, tint, 0.0f, {0, 0}, layer, false, false});
}

void SpriteBatch::draw_ex(Texture* tex, Recti src, Rect dst, f32 rotation, Vec2 origin,
                          bool flip_x, bool flip_y, i32 layer, Color tint) {
    items_.push_back({tex, src, dst, tint, rotation, origin, layer, flip_x, flip_y});
}

void SpriteBatch::end() {
    if (items_.empty()) return;

    // Sort by layer, then by texture pointer (to batch same-texture draws)
    std::sort(items_.begin(), items_.end(), [](const SpriteBatchItem& a, const SpriteBatchItem& b) {
        if (a.layer != b.layer) return a.layer < b.layer;
        return a.texture < b.texture;
    });

    sprite_count_ = static_cast<u32>(items_.size());

    // Draw items — each texture change is a new "draw call"
    Texture* last_tex = nullptr;
    for (auto& item : items_) {
        if (item.texture != last_tex) {
            draw_calls_++;
            last_tex = item.texture;
        }

        if (item.rotation != 0.0f || item.flip_x || item.flip_y) {
            draw_texture_ex(*item.texture, item.src, item.dst, item.rotation,
                           item.origin, item.flip_x, item.flip_y, item.tint);
        } else if (item.tint.r != 255 || item.tint.g != 255 || item.tint.b != 255 || item.tint.a != 255) {
            draw_texture_ex(*item.texture, item.src, item.dst, 0.0f,
                           {0, 0}, false, false, item.tint);
        } else {
            draw_texture_region(*item.texture, item.src, item.dst);
        }
    }
}

} // namespace wander
