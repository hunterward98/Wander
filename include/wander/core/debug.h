#pragma once

#include "wander/core/types.h"
#include "wander/render/text.h"

namespace wander {

// Debug overlay — shows FPS, draw calls, memory, entity count
class DebugOverlay {
public:
    void init(const Font* font);
    void update(f32 dt);
    void render();

    void set_visible(bool visible) { visible_ = visible; }
    bool visible() const { return visible_; }
    void toggle() { visible_ = !visible_; }

    // Custom stats (games can add their own)
    void set_stat(const char* name, i32 value);
    void set_stat(const char* name, f32 value);
    void set_stat(const char* name, const char* value);

private:
    const Font* font_ = nullptr;
    bool visible_ = false;

    // Stats
    f32 fps_ = 0.0f;
    f32 frame_time_ = 0.0f;
    f32 fps_update_timer_ = 0.0f;
    i32 frame_count_ = 0;

    // Custom stats (fixed-size for simplicity)
    static constexpr i32 MAX_CUSTOM_STATS = 16;
    struct CustomStat {
        char name[32] = {};
        char value[32] = {};
        bool used = false;
    };
    CustomStat custom_stats_[MAX_CUSTOM_STATS] = {};
};

} // namespace wander
