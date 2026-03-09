#include "wander/render/camera.h"
#include "wander/core/math.h"
#include <cstdlib>

namespace wander {

Vec2 Camera2D::world_to_screen(Vec2 world_pos) const {
    Vec2 offset = world_pos - position + shake_offset();
    offset = offset * zoom;
    return offset + viewport_size * 0.5f;
}

Vec2 Camera2D::screen_to_world(Vec2 screen_pos) const {
    Vec2 offset = screen_pos - viewport_size * 0.5f;
    offset = offset / zoom;
    return offset + position - shake_offset();
}

Rect Camera2D::visible_rect() const {
    Vec2 half = viewport_size * (0.5f / zoom);
    return {position.x - half.x, position.y - half.y, half.x * 2.0f, half.y * 2.0f};
}

void Camera2D::follow(Vec2 target, f32 smoothing, f32 dt) {
    position = lerp(position, target, 1.0f - std::pow(smoothing, dt));

    if (use_bounds && bounds.w > 0 && bounds.h > 0) {
        Vec2 half = viewport_size * (0.5f / zoom);
        position.x = clamp(position.x, bounds.x + half.x, bounds.right() - half.x);
        position.y = clamp(position.y, bounds.y + half.y, bounds.bottom() - half.y);
    }
}

void Camera2D::shake(f32 intensity, f32 duration) {
    shake_intensity_ = intensity;
    shake_duration_ = duration;
    shake_timer_ = 0.0f;
}

void Camera2D::update_shake(f32 dt) {
    if (shake_timer_ < shake_duration_) {
        shake_timer_ += dt;
    }
}

Vec2 Camera2D::shake_offset() const {
    if (shake_timer_ >= shake_duration_) return {0, 0};
    f32 t = 1.0f - (shake_timer_ / shake_duration_);
    f32 amount = shake_intensity_ * t;
    // Simple random offset
    f32 rx = (static_cast<f32>(std::rand()) / RAND_MAX * 2.0f - 1.0f) * amount;
    f32 ry = (static_cast<f32>(std::rand()) / RAND_MAX * 2.0f - 1.0f) * amount;
    return {rx, ry};
}

} // namespace wander
