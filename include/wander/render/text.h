#pragma once

#include "wander/core/types.h"
#include "wander/render/renderer.h"

namespace wander {

// Drop shadow settings for font rendering
struct FontShadow {
    Color color = {0, 0, 0, 0};  // alpha 0 = no shadow
    f32 offset_x = 1.0f;
    f32 offset_y = 1.0f;
};

// Font handle
struct Font {
    Texture atlas;
    void* internal = nullptr;  // stbtt_bakedchar array
    f32 size = 0.0f;
    f32 ascent = 0.0f;      // pixels from top of text to baseline (positive)
    f32 descent = 0.0f;     // pixels from baseline to bottom of descenders (positive)
    f32 line_height = 0.0f; // ascent + descent — actual height of one line of text
    i32 atlas_width = 0;
    i32 atlas_height = 0;
    FontShadow shadow;
};

// Text alignment
enum class TextAlign : u8 {
    Left,
    Center,
    Right
};

// Load a TTF font at a given pixel size
Font font_load(const char* ttf_path, f32 pixel_size);

// Destroy a font
void font_destroy(Font& font);

// Draw text at a position
void draw_text(const Font& font, const char* text, Vec2 pos, Color color = Color::white());

// Draw text with alignment within a given width
void draw_text_aligned(const Font& font, const char* text, Vec2 pos, f32 max_width,
                       TextAlign align = TextAlign::Left, Color color = Color::white());

// Measure text dimensions without drawing
Vec2 measure_text(const Font& font, const char* text);

} // namespace wander
