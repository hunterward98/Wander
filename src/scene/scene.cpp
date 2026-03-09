#include "wander/scene/scene.h"
#include "wander/render/renderer.h"
#include "wander/core/math.h"
#include "wander/core/log.h"
#include "wander/app/app.h"

namespace wander {

void SceneManager::push(std::unique_ptr<Scene> scene, Transition transition) {
    if (transition.type != TransitionType::None && !stack_.empty()) {
        transitioning_ = true;
        transition_timer_ = 0.0f;
        transition_ = transition;
        incoming_scene_ = std::move(scene);
        transition_action_ = TransitionAction::Push;
        return;
    }

    if (!stack_.empty()) {
        stack_.back().scene->on_pause();
    }
    scene->on_enter();
    stack_.push_back({std::move(scene)});
}

void SceneManager::pop(Transition transition) {
    if (stack_.empty()) return;

    if (transition.type != TransitionType::None) {
        transitioning_ = true;
        transition_timer_ = 0.0f;
        transition_ = transition;
        transition_action_ = TransitionAction::Pop;
        return;
    }

    stack_.back().scene->on_exit();
    stack_.pop_back();
    if (!stack_.empty()) {
        stack_.back().scene->on_resume();
    }
}

void SceneManager::replace(std::unique_ptr<Scene> scene, Transition transition) {
    if (transition.type != TransitionType::None && !stack_.empty()) {
        transitioning_ = true;
        transition_timer_ = 0.0f;
        transition_ = transition;
        incoming_scene_ = std::move(scene);
        transition_action_ = TransitionAction::Replace;
        return;
    }

    if (!stack_.empty()) {
        stack_.back().scene->on_exit();
        stack_.pop_back();
    }
    scene->on_enter();
    stack_.push_back({std::move(scene)});
}

void SceneManager::on_event(SDL_Event* event) {
    if (!stack_.empty() && !transitioning_) {
        stack_.back().scene->on_event(event);
    }
}

void SceneManager::update(f32 dt) {
    // Handle transition
    if (transitioning_) {
        transition_timer_ += dt;
        if (transition_timer_ >= transition_.duration) {
            transitioning_ = false;

            switch (transition_action_) {
                case TransitionAction::Push:
                    if (!stack_.empty()) stack_.back().scene->on_pause();
                    incoming_scene_->on_enter();
                    stack_.push_back({std::move(incoming_scene_)});
                    break;
                case TransitionAction::Pop:
                    stack_.back().scene->on_exit();
                    stack_.pop_back();
                    if (!stack_.empty()) stack_.back().scene->on_resume();
                    break;
                case TransitionAction::Replace:
                    if (!stack_.empty()) {
                        stack_.back().scene->on_exit();
                        stack_.pop_back();
                    }
                    incoming_scene_->on_enter();
                    stack_.push_back({std::move(incoming_scene_)});
                    break;
            }
        }
    }

    // Update current scene
    if (!stack_.empty()) {
        stack_.back().scene->on_update(dt);
    }
}

void SceneManager::render() {
    if (stack_.empty()) return;

    stack_.back().scene->on_render();

    // Draw transition overlay
    if (transitioning_) {
        f32 t = transition_timer_ / transition_.duration;
        Vec2i win = app_window_size();
        f32 w = static_cast<f32>(win.x);
        f32 h = static_cast<f32>(win.y);

        switch (transition_.type) {
            case TransitionType::Fade: {
                // Fade to black at midpoint, then fade in
                u8 alpha;
                if (t < 0.5f) {
                    alpha = static_cast<u8>(t * 2.0f * 255.0f);
                } else {
                    alpha = static_cast<u8>((1.0f - (t - 0.5f) * 2.0f) * 255.0f);
                }
                draw_rect({0, 0, w, h}, Color(0, 0, 0, alpha));
                break;
            }
            case TransitionType::SlideLeft:
            case TransitionType::SlideRight:
            case TransitionType::SlideUp:
            case TransitionType::SlideDown:
                // Simple fade for now — slide would need render-to-texture
                draw_rect({0, 0, w, h}, Color(0, 0, 0, static_cast<u8>((1.0f - t) * 200)));
                break;
            case TransitionType::None:
                break;
        }
    }
}

Scene* SceneManager::current() const {
    if (stack_.empty()) return nullptr;
    return stack_.back().scene.get();
}

} // namespace wander
