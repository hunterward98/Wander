#pragma once

#include "wander/core/types.h"
#include "wander/render/renderer.h"
#include <vector>
#include <string>

namespace wander {

// A single frame within a sprite sheet
struct SpriteFrame {
    Recti region;       // Source rect in the atlas texture
    Vec2 pivot;         // Pivot point (0-1 normalized)
    f32 duration;       // Duration in seconds (for animation)
};

// A sprite animation (sequence of frames)
struct SpriteAnimation {
    std::string name;
    std::vector<SpriteFrame> frames;
    bool looping = true;
};

// Animated sprite state
struct SpriteAnimator {
    const SpriteAnimation* current = nullptr;
    i32 frame_index = 0;
    f32 timer = 0.0f;
    bool finished = false;

    void play(const SpriteAnimation* anim);
    void update(f32 dt);
    const SpriteFrame* current_frame() const;
};

} // namespace wander
