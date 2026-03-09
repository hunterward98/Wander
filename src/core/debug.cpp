#include "wander/core/debug.h"
#include "wander/app/app.h"
#include "wander/render/renderer.h"
#include <cstdio>
#include <cstring>

namespace wander {

void DebugOverlay::init(const Font* font) {
    font_ = font;
}

void DebugOverlay::update(f32 dt) {
    frame_count_++;
    fps_update_timer_ += dt;
    frame_time_ = dt;

    if (fps_update_timer_ >= 0.5f) {
        fps_ = static_cast<f32>(frame_count_) / fps_update_timer_;
        frame_count_ = 0;
        fps_update_timer_ = 0.0f;
    }
}

void DebugOverlay::render() {
    if (!visible_ || !font_) return;

    Vec2i win = app_window_size();
    f32 w = 220.0f;
    f32 line_h = font_->size + 2.0f;
    f32 x = static_cast<f32>(win.x) - w - 10.0f;
    f32 y = 10.0f;

    // Count custom stats
    i32 custom_count = 0;
    for (i32 i = 0; i < MAX_CUSTOM_STATS; i++) {
        if (custom_stats_[i].used) custom_count++;
    }

    f32 h = line_h * (3 + custom_count) + 12.0f;

    // Background
    draw_rect({x, y, w, h}, Color(0, 0, 0, 180));
    draw_rect_outline({x, y, w, h}, Color(100, 100, 100, 150));

    f32 text_x = x + 6.0f;
    f32 text_y = y + 6.0f + font_->size * 0.8f;
    char buf[128];

    // FPS
    Color fps_color = fps_ >= 55.0f ? Color::green() :
                      fps_ >= 30.0f ? Color(255, 200, 0) : Color::red();
    snprintf(buf, sizeof(buf), "FPS: %.0f (%.1fms)", fps_, frame_time_ * 1000.0f);
    draw_text(*font_, buf, {text_x, text_y}, fps_color);
    text_y += line_h;

    // Frame count
    snprintf(buf, sizeof(buf), "Frame: %llu", static_cast<unsigned long long>(app_frame_count()));
    draw_text(*font_, buf, {text_x, text_y}, Color::white());
    text_y += line_h;

    // Window size
    snprintf(buf, sizeof(buf), "Window: %dx%d", win.x, win.y);
    draw_text(*font_, buf, {text_x, text_y}, Color::white());
    text_y += line_h;

    // Custom stats
    for (i32 i = 0; i < MAX_CUSTOM_STATS; i++) {
        if (!custom_stats_[i].used) continue;
        snprintf(buf, sizeof(buf), "%s: %s", custom_stats_[i].name, custom_stats_[i].value);
        draw_text(*font_, buf, {text_x, text_y}, Color(180, 180, 200));
        text_y += line_h;
    }
}

void DebugOverlay::set_stat(const char* name, i32 value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%d", value);
    set_stat(name, buf);
}

void DebugOverlay::set_stat(const char* name, f32 value) {
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f", value);
    set_stat(name, buf);
}

void DebugOverlay::set_stat(const char* name, const char* value) {
    // Find existing or empty slot
    for (i32 i = 0; i < MAX_CUSTOM_STATS; i++) {
        if (custom_stats_[i].used && std::strcmp(custom_stats_[i].name, name) == 0) {
            std::strncpy(custom_stats_[i].value, value, 31);
            return;
        }
    }
    for (i32 i = 0; i < MAX_CUSTOM_STATS; i++) {
        if (!custom_stats_[i].used) {
            custom_stats_[i].used = true;
            std::strncpy(custom_stats_[i].name, name, 31);
            std::strncpy(custom_stats_[i].value, value, 31);
            return;
        }
    }
}

} // namespace wander
