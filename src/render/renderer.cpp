#include "wander/render/renderer.h"
#include "wander/core/log.h"
#include <SDL3/SDL.h>
#include "stb_image.h"

namespace wander {

Texture texture_load(const char* path) {
    Texture tex;

    int w, h, channels;
    u8* pixels = stbi_load(path, &w, &h, &channels, 4); // Force RGBA
    if (!pixels) {
        LOG_ERROR("Failed to load texture: %s", path);
        return tex;
    }

    SDL_Renderer* renderer = app_get_sdl_renderer();
    SDL_Surface* surface = SDL_CreateSurfaceFrom(w, h, SDL_PIXELFORMAT_RGBA32, pixels, w * 4);
    if (!surface) {
        LOG_ERROR("Failed to create surface: %s", SDL_GetError());
        stbi_image_free(pixels);
        return tex;
    }

    tex.handle = SDL_CreateTextureFromSurface(renderer, surface);
    tex.width = w;
    tex.height = h;

    SDL_DestroySurface(surface);
    stbi_image_free(pixels);

    if (!tex.handle) {
        LOG_ERROR("Failed to create texture from surface: %s", SDL_GetError());
        return tex;
    }

    LOG_DEBUG("Loaded texture: %s (%dx%d)", path, w, h);
    return tex;
}

void texture_destroy(Texture& tex) {
    if (tex.handle) {
        SDL_DestroyTexture(tex.handle);
        tex.handle = nullptr;
        tex.width = 0;
        tex.height = 0;
    }
}

void draw_texture(const Texture& tex, Vec2 pos) {
    if (!tex.handle) return;
    SDL_FRect dst = {pos.x, pos.y, static_cast<f32>(tex.width), static_cast<f32>(tex.height)};
    SDL_RenderTexture(app_get_sdl_renderer(), tex.handle, nullptr, &dst);
}

void draw_texture_region(const Texture& tex, Recti src, Rect dst) {
    if (!tex.handle) return;
    SDL_FRect s = {static_cast<f32>(src.x), static_cast<f32>(src.y),
                   static_cast<f32>(src.w), static_cast<f32>(src.h)};
    SDL_FRect d = {dst.x, dst.y, dst.w, dst.h};
    SDL_RenderTexture(app_get_sdl_renderer(), tex.handle, &s, &d);
}

void draw_texture_ex(const Texture& tex, Recti src, Rect dst, f32 rotation,
                     Vec2 origin, bool flip_x, bool flip_y, Color tint) {
    if (!tex.handle) return;

    SDL_SetTextureColorMod(tex.handle, tint.r, tint.g, tint.b);
    SDL_SetTextureAlphaMod(tex.handle, tint.a);

    SDL_FRect s = {static_cast<f32>(src.x), static_cast<f32>(src.y),
                   static_cast<f32>(src.w), static_cast<f32>(src.h)};
    SDL_FRect d = {dst.x, dst.y, dst.w, dst.h};
    SDL_FPoint center = {origin.x, origin.y};

    SDL_FlipMode flip = SDL_FLIP_NONE;
    if (flip_x && flip_y) flip = static_cast<SDL_FlipMode>(SDL_FLIP_HORIZONTAL | SDL_FLIP_VERTICAL);
    else if (flip_x) flip = SDL_FLIP_HORIZONTAL;
    else if (flip_y) flip = SDL_FLIP_VERTICAL;

    SDL_RenderTextureRotated(app_get_sdl_renderer(), tex.handle, &s, &d, rotation, &center, flip);

    // Reset color mod
    SDL_SetTextureColorMod(tex.handle, 255, 255, 255);
    SDL_SetTextureAlphaMod(tex.handle, 255);
}

void draw_rect(Rect rect, Color color) {
    SDL_Renderer* r = app_get_sdl_renderer();
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_FRect fr = {rect.x, rect.y, rect.w, rect.h};
    SDL_RenderFillRect(r, &fr);
}

void draw_rect_outline(Rect rect, Color color) {
    SDL_Renderer* r = app_get_sdl_renderer();
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_FRect fr = {rect.x, rect.y, rect.w, rect.h};
    SDL_RenderRect(r, &fr);
}

void draw_line(Vec2 from, Vec2 to, Color color) {
    SDL_Renderer* r = app_get_sdl_renderer();
    SDL_SetRenderDrawColor(r, color.r, color.g, color.b, color.a);
    SDL_RenderLine(r, from.x, from.y, to.x, to.y);
}

void draw_shadow(Vec2 pos, f32 width, f32 height, u8 alpha, i32 segments) {
    SDL_Renderer* r = app_get_sdl_renderer();
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(r, 0, 0, 0, alpha);
    f32 hw = width * 0.5f;
    f32 hh = height * 0.5f;
    f32 scanline_h = std::max(hh / static_cast<f32>(segments), 1.0f);
    for (i32 i = -segments; i <= segments; i++) {
        f32 fy = static_cast<f32>(i) / static_cast<f32>(segments);
        f32 sy = hh * fy;
        f32 sx = hw * std::sqrt(1.0f - fy * fy);
        SDL_FRect fr = {pos.x - sx, pos.y + sy, sx * 2.0f, scanline_h};
        SDL_RenderFillRect(r, &fr);
    }
}

void set_clip_rect(Rect rect) {
    SDL_Rect cr = {static_cast<int>(rect.x), static_cast<int>(rect.y),
                   static_cast<int>(rect.w), static_cast<int>(rect.h)};
    SDL_SetRenderClipRect(app_get_sdl_renderer(), &cr);
}

void clear_clip_rect() {
    SDL_SetRenderClipRect(app_get_sdl_renderer(), nullptr);
}

} // namespace wander
