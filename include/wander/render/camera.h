#pragma once

#include "wander/core/types.h"

namespace wander {

class Camera2D {
public:
    Vec2 position;    // World position (center of viewport)
    f32 zoom = 1.0f;
    f32 rotation = 0.0f;

    // Viewport size (in pixels)
    Vec2 viewport_size;

    // Optional bounds to clamp camera position
    Rect bounds = {0, 0, 0, 0};
    bool use_bounds = false;

    // Convert world coordinates to screen coordinates
    Vec2 world_to_screen(Vec2 world_pos) const;

    // Convert screen coordinates to world coordinates
    Vec2 screen_to_world(Vec2 screen_pos) const;

    // Get the visible world rect
    Rect visible_rect() const;

    // Smooth follow a target
    void follow(Vec2 target, f32 smoothing, f32 dt);

    // Apply screen shake
    void shake(f32 intensity, f32 duration);
    void update_shake(f32 dt);
    Vec2 shake_offset() const;

private:
    f32 shake_intensity_ = 0.0f;
    f32 shake_duration_ = 0.0f;
    f32 shake_timer_ = 0.0f;
};

} // namespace wander
