#include "wander/render/text.h"
#include "wander/core/log.h"
#include <SDL3/SDL.h>
#include "stb_truetype.h"
#include <fstream>
#include <vector>
#include <cstring>

namespace wander {

// Baked character data for ASCII 32-127
static constexpr int FIRST_CHAR = 32;
static constexpr int NUM_CHARS = 96;

struct FontInternal {
    stbtt_bakedchar chars[NUM_CHARS];
};

Font font_load(const char* ttf_path, f32 pixel_size) {
    Font font;

    // Read TTF file
    std::ifstream file(ttf_path, std::ios::binary | std::ios::ate);
    if (!file) {
        LOG_ERROR("Failed to open font: %s", ttf_path);
        return font;
    }
    auto file_size = file.tellg();
    file.seekg(0);
    std::vector<u8> ttf_data(static_cast<size_t>(file_size));
    file.read(reinterpret_cast<char*>(ttf_data.data()), file_size);

    // Determine atlas size based on font size
    i32 atlas_w = 512;
    i32 atlas_h = 512;
    if (pixel_size > 32.0f) { atlas_w = 1024; atlas_h = 1024; }

    // Bake font bitmap
    auto* internal = new FontInternal();
    std::vector<u8> bitmap(atlas_w * atlas_h);

    int result = stbtt_BakeFontBitmap(ttf_data.data(), 0, pixel_size,
        bitmap.data(), atlas_w, atlas_h, FIRST_CHAR, NUM_CHARS, internal->chars);

    if (result <= 0) {
        LOG_WARN("Font atlas may be too small for size %.0f", pixel_size);
    }

    // Convert single-channel bitmap to RGBA for SDL texture
    std::vector<u8> rgba(atlas_w * atlas_h * 4);
    for (i32 i = 0; i < atlas_w * atlas_h; i++) {
        rgba[i * 4 + 0] = 255;
        rgba[i * 4 + 1] = 255;
        rgba[i * 4 + 2] = 255;
        rgba[i * 4 + 3] = bitmap[i];
    }

    // Create SDL texture
    SDL_Renderer* renderer = app_get_sdl_renderer();
    SDL_Surface* surface = SDL_CreateSurfaceFrom(atlas_w, atlas_h,
        SDL_PIXELFORMAT_RGBA32, rgba.data(), atlas_w * 4);
    if (surface) {
        font.atlas.handle = SDL_CreateTextureFromSurface(renderer, surface);
        font.atlas.width = atlas_w;
        font.atlas.height = atlas_h;
        SDL_DestroySurface(surface);

        if (font.atlas.handle) {
            SDL_SetTextureBlendMode(font.atlas.handle, SDL_BLENDMODE_BLEND);
            SDL_SetTextureScaleMode(font.atlas.handle, SDL_SCALEMODE_NEAREST);
        }
    }

    font.internal = internal;
    font.size = pixel_size;
    font.atlas_width = atlas_w;
    font.atlas_height = atlas_h;

    LOG_DEBUG("Loaded font: %s (%.0fpx, %dx%d atlas)", ttf_path, pixel_size, atlas_w, atlas_h);
    return font;
}

void font_destroy(Font& font) {
    texture_destroy(font.atlas);
    delete static_cast<FontInternal*>(font.internal);
    font.internal = nullptr;
}

// Render text glyphs without setting color mod (caller handles that)
static void draw_text_raw(const Font& font, const char* text, Vec2 pos) {
    auto* internal = static_cast<FontInternal*>(font.internal);
    f32 x = pos.x;
    f32 y = pos.y;

    while (*text) {
        if (*text == '\n') {
            x = pos.x;
            y += font.size;
            text++;
            continue;
        }

        int c = static_cast<int>(*text) - FIRST_CHAR;
        if (c >= 0 && c < NUM_CHARS) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(internal->chars, font.atlas_width, font.atlas_height,
                c, &x, &y, &q, 1);

            SDL_FRect src = {q.s0 * font.atlas_width, q.t0 * font.atlas_height,
                            (q.s1 - q.s0) * font.atlas_width, (q.t1 - q.t0) * font.atlas_height};
            SDL_FRect dst = {q.x0, q.y0, q.x1 - q.x0, q.y1 - q.y0};
            SDL_RenderTexture(app_get_sdl_renderer(), font.atlas.handle, &src, &dst);
        }
        text++;
    }
}

void draw_text(const Font& font, const char* text, Vec2 pos, Color color) {
    if (!font.atlas.handle || !font.internal || !text) return;

    // Drop shadow pass
    if (font.shadow.color.a > 0) {
        SDL_SetTextureColorMod(font.atlas.handle, font.shadow.color.r, font.shadow.color.g, font.shadow.color.b);
        SDL_SetTextureAlphaMod(font.atlas.handle, font.shadow.color.a);
        draw_text_raw(font, text, {pos.x + font.shadow.offset_x, pos.y + font.shadow.offset_y});
    }

    // Foreground pass
    SDL_SetTextureColorMod(font.atlas.handle, color.r, color.g, color.b);
    SDL_SetTextureAlphaMod(font.atlas.handle, color.a);
    draw_text_raw(font, text, pos);

    SDL_SetTextureColorMod(font.atlas.handle, 255, 255, 255);
    SDL_SetTextureAlphaMod(font.atlas.handle, 255);
}

void draw_text_aligned(const Font& font, const char* text, Vec2 pos, f32 max_width,
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

Vec2 measure_text(const Font& font, const char* text) {
    if (!font.internal || !text) return {0, 0};

    auto* internal = static_cast<FontInternal*>(font.internal);
    f32 x = 0, y = 0;
    f32 max_x = 0;
    i32 lines = 1;

    while (*text) {
        if (*text == '\n') {
            if (x > max_x) max_x = x;
            x = 0;
            lines++;
            text++;
            continue;
        }

        int c = static_cast<int>(*text) - FIRST_CHAR;
        if (c >= 0 && c < NUM_CHARS) {
            stbtt_aligned_quad q;
            stbtt_GetBakedQuad(internal->chars, font.atlas_width, font.atlas_height,
                c, &x, &y, &q, 1);
        }
        text++;
    }

    if (x > max_x) max_x = x;
    return {max_x, font.size * lines};
}

} // namespace wander
