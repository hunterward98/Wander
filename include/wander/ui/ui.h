#pragma once

#include "wander/core/types.h"
#include "wander/render/text.h"
#include "wander/render/renderer.h"
#include "wander/input/input.h"

namespace wander {

// UI style configuration
struct UIStyle {
    Color bg_normal      = Color(40, 40, 50, 220);
    Color bg_hover       = Color(60, 60, 80, 230);
    Color bg_active      = Color(80, 100, 140, 240);
    Color border         = Color(100, 100, 120, 200);
    Color text_normal    = Color::white();
    Color text_disabled  = Color(120, 120, 120, 200);
    Color slider_fill    = Color(80, 140, 220, 255);
    Color progress_fill  = Color(60, 180, 80, 255);
    Color panel_bg       = Color(25, 25, 35, 230);
    f32 padding = 8.0f;
    f32 border_width = 1.0f;
    f32 item_spacing = 4.0f;
    f32 min_touch_size = 44.0f;  // Apple HIG minimum
};

// UI context — manages immediate-mode UI state
class UI {
public:
    void init(const Font* font);
    void begin(Vec2 mouse_pos, bool mouse_down);
    void end();

    // Panels
    void begin_panel(const char* title, Rect bounds);
    void end_panel();

    // Widgets (auto-layout inside panels, or manual position)
    bool button(const char* label, Rect bounds = {});
    bool icon_button(const Texture& tex, Recti region, Rect bounds = {});
    void label(const char* text, Color color = Color::white());
    bool slider(const char* label, f32* value, f32 min_val, f32 max_val);
    void progress_bar(f32 fraction, Rect bounds = {});
    bool checkbox(const char* label, bool* checked);
    bool text_input(const char* label, char* buffer, i32 buffer_size);
    void separator();
    void spacing(f32 amount);
    void image(const Texture& tex, Recti region, Vec2 size);

    // Tooltip (shown next frame if hovered)
    void tooltip(const char* text);

    // Layout helpers
    void same_line();
    void indent(f32 amount = 16.0f);
    void unindent(f32 amount = 16.0f);

    // Style
    UIStyle& style() { return style_; }

    // Query
    bool is_hovered() const { return hovered_; }
    bool any_active() const { return active_id_ != 0; }

private:
    u32 gen_id(const char* label);
    Rect next_rect(f32 width, f32 height);
    bool mouse_in_rect(Rect r) const;

    const Font* font_ = nullptr;
    UIStyle style_;

    Vec2 mouse_pos_;
    bool mouse_down_ = false;
    bool mouse_was_down_ = false;

    u32 hot_id_ = 0;      // Hovered widget
    u32 active_id_ = 0;   // Clicked/dragged widget
    bool hovered_ = false;

    // Auto-layout state
    Rect panel_rect_;
    Vec2 cursor_;
    f32 indent_ = 0.0f;
    bool same_line_ = false;
    f32 last_item_height_ = 0.0f;
    f32 last_item_width_ = 0.0f;

    // Tooltip
    const char* tooltip_text_ = nullptr;
    Vec2 tooltip_pos_;
};

} // namespace wander
