#include "wander/render/bitmap_font.h"
#include "wander/core/log.h"
#include <SDL3/SDL.h>
#include <json.hpp>
#include <fstream>
#include <string>

namespace wander {

BitmapFont bitmap_font_load(const char* atlas_png, const char* meta_json, f32 scale) {
    BitmapFont font;

    // Load atlas texture
    font.atlas = texture_load(atlas_png);
    if (!font.atlas.handle) {
        LOG_ERROR("Failed to load bitmap font atlas: %s", atlas_png);
        return font;
    }

    // Enable blending for color modulation
    SDL_SetTextureBlendMode(font.atlas.handle, SDL_BLENDMODE_BLEND);

    // Load JSON metadata
    std::ifstream file(meta_json);
    if (!file) {
        LOG_ERROR("Failed to open bitmap font metadata: %s", meta_json);
        texture_destroy(font.atlas);
        return font;
    }

    nlohmann::json j;
    file >> j;

    font.glyph_w = j.value("glyph_w", 8);
    font.glyph_h = j.value("glyph_h", 12);
    font.first_char = j.value("first_char", 32);
    font.num_chars = j.value("num_chars", 96);
    font.cols = j.value("cols", 16);
    font.scale = scale;

    // Default advances to glyph_w
    for (i32 i = 0; i < 128; i++) {
        font.advances[i] = font.glyph_w;
    }

    // Parse per-char advances from JSON
    if (j.contains("advances")) {
        for (auto& [key, val] : j["advances"].items()) {
            i32 ascii = std::stoi(key);
            if (ascii >= 0 && ascii < 128) {
                font.advances[ascii] = val.get<i32>();
            }
        }
    }

    font.loaded = true;
    LOG_DEBUG("Loaded bitmap font: %s (%dx%d glyphs, scale %.1f)",
              atlas_png, font.glyph_w, font.glyph_h, scale);
    return font;
}

void bitmap_font_destroy(BitmapFont& font) {
    texture_destroy(font.atlas);
    font.loaded = false;
}

void draw_text(const BitmapFont& font, const char* text, Vec2 pos, Color color) {
    if (!font.loaded || !font.atlas.handle || !text) return;

    SDL_SetTextureColorMod(font.atlas.handle, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font.atlas.handle, color.a);

    f32 x = pos.x;
    f32 y = pos.y;
    f32 s = font.scale;

    while (*text) {
        if (*text == '\n') {
            x = pos.x;
            y += font.glyph_h * s;
            text++;
            continue;
        }

        i32 c = static_cast<i32>(*text);
        i32 idx = c - font.first_char;
        if (idx >= 0 && idx < font.num_chars) {
            i32 col = idx % font.cols;
            i32 row = idx / font.cols;

            Recti src;
            src.x = col * font.glyph_w;
            src.y = row * font.glyph_h;
            src.w = font.glyph_w;
            src.h = font.glyph_h;

            Rect dst;
            dst.x = x;
            dst.y = y;
            dst.w = font.glyph_w * s;
            dst.h = font.glyph_h * s;

            draw_texture_region(font.atlas, src, dst);

            // Advance by per-char width
            i32 adv = (c >= 0 && c < 128) ? font.advances[c] : font.glyph_w;
            x += adv * s;
        } else {
            // Unknown char — advance by glyph width
            x += font.glyph_w * s;
        }
        text++;
    }

    SDL_SetTextureColorMod(font.atlas.handle, 255, 255, 255);
    SDL_SetTextureAlphaMod(font.atlas.handle, 255);
}

void draw_text_aligned(const BitmapFont& font, const char* text, Vec2 pos, f32 max_width,
                       TextAlign align, Color color) {
    if (align == TextAlign::Left) {
        draw_text(font, text, pos, color);
        return;
    }

    Vec2 size = measure_text(font, text);
    if (align == TextAlign::Center) {
        pos.x += (max_width - size.x) * 0.5f;
    } else if (align == TextAlign::Right) {
        pos.x += max_width - size.x;
    }
    draw_text(font, text, pos, color);
}

Vec2 measure_text(const BitmapFont& font, const char* text) {
    if (!font.loaded || !text) return {0, 0};

    f32 x = 0;
    f32 max_x = 0;
    i32 lines = 1;
    f32 s = font.scale;

    while (*text) {
        if (*text == '\n') {
            if (x > max_x) max_x = x;
            x = 0;
            lines++;
            text++;
            continue;
        }

        i32 c = static_cast<i32>(*text);
        i32 adv = (c >= 0 && c < 128) ? font.advances[c] : font.glyph_w;
        x += adv * s;
        text++;
    }

    if (x > max_x) max_x = x;
    return {max_x, font.glyph_h * s * lines};
}

} // namespace wander
