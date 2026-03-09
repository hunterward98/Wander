#pragma once

#include "wander/core/types.h"
#include <cmath>
#include <algorithm>

namespace wander {

constexpr f32 PI = 3.14159265358979323846f;
constexpr f32 TAU = PI * 2.0f;
constexpr f32 DEG2RAD = PI / 180.0f;
constexpr f32 RAD2DEG = 180.0f / PI;

// Vec2 math functions
inline f32 dot(Vec2 a, Vec2 b) { return a.x * b.x + a.y * b.y; }
inline f32 cross(Vec2 a, Vec2 b) { return a.x * b.y - a.y * b.x; }
inline f32 length(Vec2 v) { return std::sqrt(dot(v, v)); }
inline f32 length_sq(Vec2 v) { return dot(v, v); }
inline f32 distance(Vec2 a, Vec2 b) { return length(b - a); }
inline f32 distance_sq(Vec2 a, Vec2 b) { return length_sq(b - a); }

inline Vec2 normalize(Vec2 v) {
    f32 len = length(v);
    return (len > 0.0001f) ? v / len : Vec2{0.0f, 0.0f};
}

inline Vec2 lerp(Vec2 a, Vec2 b, f32 t) {
    return a + (b - a) * t;
}

inline Vec2 rotate(Vec2 v, f32 radians) {
    f32 c = std::cos(radians);
    f32 s = std::sin(radians);
    return {v.x * c - v.y * s, v.x * s + v.y * c};
}

// Scalar math
inline f32 clamp(f32 v, f32 lo, f32 hi) { return std::max(lo, std::min(v, hi)); }
inline f32 lerp(f32 a, f32 b, f32 t) { return a + (b - a) * t; }
inline f32 remap(f32 v, f32 from_lo, f32 from_hi, f32 to_lo, f32 to_hi) {
    f32 t = (v - from_lo) / (from_hi - from_lo);
    return lerp(to_lo, to_hi, t);
}
inline f32 approach(f32 current, f32 target, f32 delta) {
    if (current < target) return std::min(current + delta, target);
    if (current > target) return std::max(current - delta, target);
    return target;
}

inline i32 clamp(i32 v, i32 lo, i32 hi) { return std::max(lo, std::min(v, hi)); }

// Simple 3x3 matrix for 2D transforms (row-major)
struct Mat3 {
    f32 m[9] = {1, 0, 0, 0, 1, 0, 0, 0, 1};

    static Mat3 identity() { return {}; }

    static Mat3 translate(Vec2 t) {
        Mat3 r;
        r.m[6] = t.x;
        r.m[7] = t.y;
        return r;
    }

    static Mat3 scale(Vec2 s) {
        Mat3 r;
        r.m[0] = s.x;
        r.m[4] = s.y;
        return r;
    }

    static Mat3 rotation(f32 radians) {
        Mat3 r;
        f32 c = std::cos(radians);
        f32 s = std::sin(radians);
        r.m[0] = c; r.m[1] = -s;
        r.m[3] = s; r.m[4] = c;
        return r;
    }

    Vec2 transform_point(Vec2 p) const {
        return {
            m[0] * p.x + m[1] * p.y + m[6],
            m[3] * p.x + m[4] * p.y + m[7]
        };
    }

    // Row-major multiply: this * other
    Mat3 operator*(const Mat3& o) const {
        Mat3 r;
        for (int row = 0; row < 3; row++) {
            for (int col = 0; col < 3; col++) {
                r.m[row * 3 + col] =
                    m[row * 3 + 0] * o.m[0 * 3 + col] +
                    m[row * 3 + 1] * o.m[1 * 3 + col] +
                    m[row * 3 + 2] * o.m[2 * 3 + col];
            }
        }
        return r;
    }
};

} // namespace wander
