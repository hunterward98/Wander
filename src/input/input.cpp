#include "wander/input/input.h"
#include "wander/core/log.h"
#include <SDL3/SDL.h>

namespace wander {

void InputManager::init() {
    LOG_INFO("Input system initialized");
}

void InputManager::shutdown() {
    actions_.clear();
    key_bindings_.clear();
    mouse_bindings_.clear();
}

void InputManager::update() {
    // Clear per-frame flags
    for (auto& [name, state] : actions_) {
        state.pressed = false;
        state.released = false;
    }
    mouse_delta_ = {0, 0};
    wheel_x_ = 0;
    wheel_y_ = 0;
}

void InputManager::process_event(const SDL_Event& event) {
    switch (event.type) {
        case SDL_EVENT_KEY_DOWN:
            if (!event.key.repeat) {
                for (auto& bind : key_bindings_) {
                    if (bind.keycode == static_cast<i32>(event.key.key)) {
                        auto& state = actions_[bind.action];
                        state.down = true;
                        state.pressed = true;
                    }
                }
            }
            break;

        case SDL_EVENT_KEY_UP:
            for (auto& bind : key_bindings_) {
                if (bind.keycode == static_cast<i32>(event.key.key)) {
                    auto& state = actions_[bind.action];
                    state.down = false;
                    state.released = true;
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_DOWN:
            for (auto& bind : mouse_bindings_) {
                if (bind.button == event.button.button) {
                    auto& state = actions_[bind.action];
                    state.down = true;
                    state.pressed = true;
                }
            }
            break;

        case SDL_EVENT_MOUSE_BUTTON_UP:
            for (auto& bind : mouse_bindings_) {
                if (bind.button == event.button.button) {
                    auto& state = actions_[bind.action];
                    state.down = false;
                    state.released = true;
                }
            }
            break;

        case SDL_EVENT_MOUSE_MOTION:
            mouse_pos_ = {event.motion.x, event.motion.y};
            mouse_delta_ = {event.motion.xrel, event.motion.yrel};
            break;

        case SDL_EVENT_MOUSE_WHEEL: {
            f32 mult = (event.wheel.direction == SDL_MOUSEWHEEL_FLIPPED) ? -1.0f : 1.0f;
            wheel_x_ += event.wheel.x * mult;
            wheel_y_ += event.wheel.y * mult;
            break;
        }

        default:
            break;
    }
}

void InputManager::bind_key(const std::string& action, i32 sdl_keycode) {
    key_bindings_.push_back({action, sdl_keycode});
    actions_[action] = {};
}

void InputManager::bind_mouse(const std::string& action, u8 button) {
    mouse_bindings_.push_back({action, button});
    actions_[action] = {};
}

bool InputManager::pressed(const std::string& action) const {
    auto it = actions_.find(action);
    return it != actions_.end() && it->second.pressed;
}

bool InputManager::held(const std::string& action) const {
    auto it = actions_.find(action);
    return it != actions_.end() && it->second.down;
}

bool InputManager::released(const std::string& action) const {
    auto it = actions_.find(action);
    return it != actions_.end() && it->second.released;
}

} // namespace wander
