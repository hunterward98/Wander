#pragma once

#include "wander/core/types.h"

struct SDL_Renderer;
struct SDL_Texture;

namespace wander {

// Forward declare — implemented in app.cpp
SDL_Renderer* app_get_sdl_renderer();

// Texture handle (wraps SDL_Texture)
struct Texture {
    SDL_Texture* handle = nullptr;
    i32 width = 0;
    i32 height = 0;
};

// Load a texture from a file (PNG, JPG, BMP via stb_image)
Texture texture_load(const char* path);

// Destroy a texture
void texture_destroy(Texture& tex);

// Draw a texture at a position
void draw_texture(const Texture& tex, Vec2 pos);

// Draw a texture region (from sprite sheet) at a position
void draw_texture_region(const Texture& tex, Recti src, Rect dst);

// Draw a texture with full transform
void draw_texture_ex(const Texture& tex, Recti src, Rect dst, f32 rotation, Vec2 origin, bool flip_x, bool flip_y, Color tint);

// Draw a filled rectangle
void draw_rect(Rect rect, Color color);

// Draw a rectangle outline
void draw_rect_outline(Rect rect, Color color);

// Draw a drop shadow (filled ellipse) at a position
// pos: center of the shadow
// width, height: ellipse dimensions
// alpha: shadow opacity (0-255)
// segments: number of horizontal scanlines for smoothness
void draw_shadow(Vec2 pos, f32 width, f32 height, u8 alpha = 40, i32 segments = 16);

} // namespace wander
