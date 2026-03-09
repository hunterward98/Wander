#pragma once

#include <cstdint>
#include <cstddef>

namespace wander {

// Fixed-width integer types
using u8  = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8  = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

// 2D vector (float)
struct Vec2 {
    f32 x = 0.0f;
    f32 y = 0.0f;

    Vec2() = default;
    Vec2(f32 x, f32 y) : x(x), y(y) {}

    Vec2 operator+(const Vec2& r) const { return {x + r.x, y + r.y}; }
    Vec2 operator-(const Vec2& r) const { return {x - r.x, y - r.y}; }
    Vec2 operator*(f32 s) const { return {x * s, y * s}; }
    Vec2 operator/(f32 s) const { return {x / s, y / s}; }
    Vec2& operator+=(const Vec2& r) { x += r.x; y += r.y; return *this; }
    Vec2& operator-=(const Vec2& r) { x -= r.x; y -= r.y; return *this; }
    Vec2& operator*=(f32 s) { x *= s; y *= s; return *this; }
    bool operator==(const Vec2& r) const { return x == r.x && y == r.y; }
    bool operator!=(const Vec2& r) const { return !(*this == r); }
};

inline Vec2 operator*(f32 s, const Vec2& v) { return {v.x * s, v.y * s}; }

// 2D vector (integer, for pixel coords / tile coords)
struct Vec2i {
    i32 x = 0;
    i32 y = 0;

    Vec2i() = default;
    Vec2i(i32 x, i32 y) : x(x), y(y) {}

    Vec2i operator+(const Vec2i& r) const { return {x + r.x, y + r.y}; }
    Vec2i operator-(const Vec2i& r) const { return {x - r.x, y - r.y}; }
    Vec2i operator*(i32 s) const { return {x * s, y * s}; }
    bool operator==(const Vec2i& r) const { return x == r.x && y == r.y; }
    bool operator!=(const Vec2i& r) const { return !(*this == r); }

    Vec2 to_vec2() const { return {static_cast<f32>(x), static_cast<f32>(y)}; }
};

// Axis-aligned bounding box
struct Rect {
    f32 x = 0.0f;
    f32 y = 0.0f;
    f32 w = 0.0f;
    f32 h = 0.0f;

    Rect() = default;
    Rect(f32 x, f32 y, f32 w, f32 h) : x(x), y(y), w(w), h(h) {}

    f32 right() const { return x + w; }
    f32 bottom() const { return y + h; }
    Vec2 center() const { return {x + w * 0.5f, y + h * 0.5f}; }
    bool contains(Vec2 p) const { return p.x >= x && p.x < right() && p.y >= y && p.y < bottom(); }
    bool overlaps(const Rect& r) const {
        return x < r.right() && right() > r.x && y < r.bottom() && bottom() > r.y;
    }
};

// Integer rect (for sprite atlas regions)
struct Recti {
    i32 x = 0;
    i32 y = 0;
    i32 w = 0;
    i32 h = 0;

    Recti() = default;
    Recti(i32 x, i32 y, i32 w, i32 h) : x(x), y(y), w(w), h(h) {}
};

// RGBA color
struct Color {
    u8 r = 255;
    u8 g = 255;
    u8 b = 255;
    u8 a = 255;

    Color() = default;
    Color(u8 r, u8 g, u8 b, u8 a = 255) : r(r), g(g), b(b), a(a) {}

    static Color white()       { return {255, 255, 255, 255}; }
    static Color black()       { return {0, 0, 0, 255}; }
    static Color red()         { return {255, 0, 0, 255}; }
    static Color green()       { return {0, 255, 0, 255}; }
    static Color blue()        { return {0, 0, 255, 255}; }
    static Color transparent() { return {0, 0, 0, 0}; }
};

} // namespace wander
