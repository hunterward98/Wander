#include "wander/ui/ui.h"
#include "wander/core/math.h"
#include <cstring>

namespace wander {

// Simple hash for widget IDs
static u32 hash_string(const char* str) {
    u32 hash = 2166136261u;
    while (*str) {
        hash ^= static_cast<u32>(*str++);
        hash *= 16777619u;
    }
    return hash;
}

void UI::init(const Font* font) {
    font_ = font;
}

void UI::begin(Vec2 mouse_pos, bool mouse_down) {
    mouse_was_down_ = mouse_down_;
    mouse_pos_ = mouse_pos;
    mouse_down_ = mouse_down;
    hot_id_ = 0;
    hovered_ = false;
    tooltip_text_ = nullptr;
}

void UI::end() {
    if (!mouse_down_) {
        active_id_ = 0;
    }

    // Draw tooltip
    if (tooltip_text_ && font_) {
        Vec2 size = measure_text(*font_, tooltip_text_);
        Rect bg = {tooltip_pos_.x + 12, tooltip_pos_.y + 12,
                   size.x + style_.padding * 2, size.y + style_.padding * 2};
        draw_rect(bg, Color(20, 20, 30, 240));
        draw_rect_outline(bg, style_.border);
        draw_text(*font_, tooltip_text_,
            {bg.x + style_.padding, bg.y + style_.padding},
            style_.text_normal);
    }
}

void UI::begin_panel(const char* title, Rect bounds) {
    panel_rect_ = bounds;
    cursor_ = {bounds.x + style_.padding, bounds.y + style_.padding};
    indent_ = 0.0f;
    same_line_ = false;

    // Draw panel background
    draw_rect(bounds, style_.panel_bg);
    draw_rect_outline(bounds, style_.border);

    // Draw title
    if (title && title[0] && font_) {
        draw_text(*font_, title,
            {cursor_.x, cursor_.y}, style_.text_normal);
        cursor_.y += font_->size + style_.item_spacing;
        separator();
    }
}

void UI::end_panel() {
    // Nothing to do for immediate mode
}

bool UI::button(const char* label, Rect bounds) {
    u32 id = gen_id(label);

    if (bounds.w == 0 && bounds.h == 0) {
        Vec2 text_size = font_ ? measure_text(*font_, label) : Vec2{80, 20};
        f32 w = text_size.x + style_.padding * 2;
        f32 h = fmaxf(text_size.y + style_.padding * 2, style_.min_touch_size);
        bounds = next_rect(w, h);
    }

    bool hover = mouse_in_rect(bounds);
    bool pressed = false;

    if (hover) {
        hot_id_ = id;
        hovered_ = true;
        if (mouse_down_ && !mouse_was_down_) {
            active_id_ = id;
        }
        if (!mouse_down_ && active_id_ == id) {
            pressed = true;
        }
    }

    // Draw
    Color bg = style_.bg_normal;
    if (active_id_ == id) bg = style_.bg_active;
    else if (hover) bg = style_.bg_hover;

    draw_rect(bounds, bg);
    draw_rect_outline(bounds, style_.border);

    if (font_ && label) {
        Vec2 text_size = measure_text(*font_, label);
        Vec2 text_pos = {
            bounds.x + (bounds.w - text_size.x) * 0.5f,
            bounds.y + (bounds.h - text_size.y) * 0.5f
        };
        draw_text(*font_, label, text_pos, style_.text_normal);
    }

    return pressed;
}

bool UI::icon_button(const Texture& tex, Recti region, Rect bounds) {
    static u32 icon_counter = 0;
    char id_str[32];
    snprintf(id_str, sizeof(id_str), "icon_%u", icon_counter++);
    u32 id = gen_id(id_str);

    if (bounds.w == 0 && bounds.h == 0) {
        bounds = next_rect(style_.min_touch_size, style_.min_touch_size);
    }

    bool hover = mouse_in_rect(bounds);
    bool pressed = false;

    if (hover) {
        hot_id_ = id;
        hovered_ = true;
        if (mouse_down_ && !mouse_was_down_) active_id_ = id;
        if (!mouse_down_ && active_id_ == id) pressed = true;
    }

    Color bg = style_.bg_normal;
    if (active_id_ == id) bg = style_.bg_active;
    else if (hover) bg = style_.bg_hover;

    draw_rect(bounds, bg);
    draw_rect_outline(bounds, style_.border);

    // Center icon in button
    f32 icon_w = static_cast<f32>(region.w);
    f32 icon_h = static_cast<f32>(region.h);
    Rect icon_dst = {
        bounds.x + (bounds.w - icon_w) * 0.5f,
        bounds.y + (bounds.h - icon_h) * 0.5f,
        icon_w, icon_h
    };
    draw_texture_region(tex, region, icon_dst);

    return pressed;
}

void UI::label(const char* text, Color color) {
    if (!font_ || !text) return;
    Vec2 size = measure_text(*font_, text);
    Rect r = next_rect(size.x, size.y);
    draw_text(*font_, text, {r.x, r.y}, color);
}

bool UI::slider(const char* label_text, f32* value, f32 min_val, f32 max_val) {
    u32 id = gen_id(label_text);

    f32 slider_w = panel_rect_.w - style_.padding * 2 - indent_;
    f32 slider_h = 24.0f;
    Rect bounds = next_rect(slider_w, slider_h);

    bool hover = mouse_in_rect(bounds);
    bool changed = false;

    if (hover) {
        hot_id_ = id;
        hovered_ = true;
        if (mouse_down_ && !mouse_was_down_) active_id_ = id;
    }

    if (active_id_ == id && mouse_down_) {
        f32 t = clamp((mouse_pos_.x - bounds.x) / bounds.w, 0.0f, 1.0f);
        f32 new_val = lerp(min_val, max_val, t);
        if (new_val != *value) {
            *value = new_val;
            changed = true;
        }
    }

    // Draw track
    draw_rect(bounds, style_.bg_normal);
    draw_rect_outline(bounds, style_.border);

    // Draw fill
    f32 t = clamp((*value - min_val) / (max_val - min_val), 0.0f, 1.0f);
    Rect fill = {bounds.x, bounds.y, bounds.w * t, bounds.h};
    draw_rect(fill, style_.slider_fill);

    // Draw label
    if (font_ && label_text) {
        char buf[128];
        snprintf(buf, sizeof(buf), "%s: %.1f", label_text, *value);
        Vec2 text_size = measure_text(*font_, buf);
        draw_text(*font_, buf,
            {bounds.x + (bounds.w - text_size.x) * 0.5f,
             bounds.y + (bounds.h - text_size.y) * 0.5f},
            style_.text_normal);
    }

    return changed;
}

void UI::progress_bar(f32 fraction, Rect bounds) {
    if (bounds.w == 0 && bounds.h == 0) {
        f32 w = panel_rect_.w - style_.padding * 2 - indent_;
        bounds = next_rect(w, 20.0f);
    }

    fraction = clamp(fraction, 0.0f, 1.0f);
    draw_rect(bounds, style_.bg_normal);
    draw_rect({bounds.x, bounds.y, bounds.w * fraction, bounds.h}, style_.progress_fill);
    draw_rect_outline(bounds, style_.border);
}

bool UI::checkbox(const char* label_text, bool* checked) {
    u32 id = gen_id(label_text);

    f32 box_size = fmaxf(20.0f, style_.min_touch_size * 0.5f);
    Vec2 text_size = font_ ? measure_text(*font_, label_text) : Vec2{0, 0};
    f32 total_w = box_size + style_.padding + text_size.x;
    Rect bounds = next_rect(total_w, fmaxf(box_size, text_size.y));

    Rect box_rect = {bounds.x, bounds.y + (bounds.h - box_size) * 0.5f, box_size, box_size};
    bool hover = mouse_in_rect(bounds);
    bool toggled = false;

    if (hover) {
        hot_id_ = id;
        hovered_ = true;
        if (mouse_down_ && !mouse_was_down_) active_id_ = id;
        if (!mouse_down_ && active_id_ == id) {
            *checked = !*checked;
            toggled = true;
        }
    }

    Color bg = hover ? style_.bg_hover : style_.bg_normal;
    draw_rect(box_rect, bg);
    draw_rect_outline(box_rect, style_.border);

    if (*checked) {
        Rect inner = {box_rect.x + 4, box_rect.y + 4, box_rect.w - 8, box_rect.h - 8};
        draw_rect(inner, style_.slider_fill);
    }

    if (font_ && label_text) {
        draw_text(*font_, label_text,
            {box_rect.right() + style_.padding,
             bounds.y + (bounds.h - measure_text(*font_, label_text).y) * 0.5f},
            style_.text_normal);
    }

    return toggled;
}

bool UI::text_input(const char* label_text, char* buffer, i32 buffer_size) {
    // Simplified text input — full implementation would handle keyboard events
    u32 id = gen_id(label_text);
    (void)id; (void)buffer; (void)buffer_size;

    if (font_ && label_text) {
        label(label_text);
    }

    f32 w = panel_rect_.w - style_.padding * 2 - indent_;
    Rect bounds = next_rect(w, font_ ? font_->size + style_.padding * 2 : 30.0f);

    draw_rect(bounds, Color(15, 15, 20, 240));
    draw_rect_outline(bounds, style_.border);

    if (font_ && buffer[0]) {
        draw_text(*font_, buffer,
            {bounds.x + style_.padding, bounds.y + style_.padding},
            style_.text_normal);
    }

    return false; // TODO: keyboard input handling
}

void UI::separator() {
    Rect r = next_rect(panel_rect_.w - style_.padding * 2 - indent_, 1.0f);
    draw_rect(r, style_.border);
    cursor_.y += style_.item_spacing;
}

void UI::spacing(f32 amount) {
    cursor_.y += amount;
}

void UI::image(const Texture& tex, Recti region, Vec2 size) {
    Rect bounds = next_rect(size.x, size.y);
    draw_texture_region(tex, region, bounds);
}

void UI::tooltip(const char* text) {
    if (hot_id_ != 0) {
        tooltip_text_ = text;
        tooltip_pos_ = mouse_pos_;
    }
}

void UI::same_line() {
    same_line_ = true;
}

void UI::indent(f32 amount) {
    indent_ += amount;
    cursor_.x += amount;
}

void UI::unindent(f32 amount) {
    indent_ -= amount;
    cursor_.x -= amount;
}

u32 UI::gen_id(const char* label) {
    return hash_string(label);
}

Rect UI::next_rect(f32 width, f32 height) {
    if (same_line_) {
        cursor_.x += last_item_width_ + style_.item_spacing;
        same_line_ = false;
    } else {
        cursor_.x = panel_rect_.x + style_.padding + indent_;
    }

    Rect r = {cursor_.x, cursor_.y, width, height};
    last_item_width_ = width;
    last_item_height_ = height;

    if (!same_line_) {
        cursor_.y += height + style_.item_spacing;
    }

    return r;
}

bool UI::mouse_in_rect(Rect r) const {
    return r.contains(mouse_pos_);
}

} // namespace wander
