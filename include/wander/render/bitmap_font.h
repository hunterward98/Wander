#pragma once

#include "wander/core/types.h"
#include "wander/render/renderer.h"
#include "wander/render/text.h"

namespace wander {

// Bitmap font loaded from a pixel art atlas PNG + JSON metadata
struct BitmapFont {
    Texture atlas;
    i32 glyph_w = 0;      // cell width in atlas
    i32 glyph_h = 0;      // cell height in atlas
    i32 first_char = 32;
    i32 num_chars = 96;
    i32 cols = 16;
    i32 advances[128] = {};  // per-char advance width (indexed by ASCII)
    f32 scale = 1.0f;        // render scale multiplier
    bool loaded = false;
};

// Load a bitmap font from atlas PNG + metadata JSON
BitmapFont bitmap_font_load(const char* atlas_png, const char* meta_json, f32 scale = 1.0f);

// Destroy a bitmap font
void bitmap_font_destroy(BitmapFont& font);

// Draw text using bitmap font
void draw_text(const BitmapFont& font, const char* text, Vec2 pos, Color color = Color::white());

// Draw text with alignment within a given width
void draw_text_aligned(const BitmapFont& font, const char* text, Vec2 pos, f32 max_width,
                       TextAlign align = TextAlign::Left, Color color = Color::white());

// Measure text dimensions without drawing
Vec2 measure_text(const BitmapFont& font, const char* text);

} // namespace wander
