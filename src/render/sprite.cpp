#include "wander/render/sprite.h"

namespace wander {

void SpriteAnimator::play(const SpriteAnimation* anim) {
    if (current == anim) return;
    current = anim;
    frame_index = 0;
    timer = 0.0f;
    finished = false;
}

void SpriteAnimator::update(f32 dt) {
    if (!current || finished) return;

    timer += dt;
    const auto& frame = current->frames[frame_index];
    if (timer >= frame.duration) {
        timer -= frame.duration;
        frame_index++;
        if (frame_index >= static_cast<i32>(current->frames.size())) {
            if (current->looping) {
                frame_index = 0;
            } else {
                frame_index = static_cast<i32>(current->frames.size()) - 1;
                finished = true;
            }
        }
    }
}

const SpriteFrame* SpriteAnimator::current_frame() const {
    if (!current || current->frames.empty()) return nullptr;
    return &current->frames[frame_index];
}

} // namespace wander
