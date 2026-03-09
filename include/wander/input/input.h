#pragma once

#include "wander/core/types.h"
#include <string>
#include <unordered_map>
#include <vector>

union SDL_Event;

namespace wander {

// Input actions — game code binds keys/buttons to named actions
class InputManager {
public:
    void init();
    void shutdown();
    void update();             // Call at start of frame (resets just-pressed/released)
    void process_event(const SDL_Event& event);

    // Action binding
    void bind_key(const std::string& action, i32 sdl_keycode);
    void bind_mouse(const std::string& action, u8 button);

    // Query
    bool pressed(const std::string& action) const;   // Just pressed this frame
    bool held(const std::string& action) const;       // Currently held
    bool released(const std::string& action) const;   // Just released this frame

    // Raw mouse
    Vec2 mouse_pos() const { return mouse_pos_; }
    Vec2 mouse_delta() const { return mouse_delta_; }
    f32 mouse_wheel_x() const { return wheel_x_; }
    f32 mouse_wheel_y() const { return wheel_y_; }

private:
    struct ActionState {
        bool down = false;
        bool pressed = false;
        bool released = false;
    };

    struct KeyBinding {
        std::string action;
        i32 keycode;
    };

    struct MouseBinding {
        std::string action;
        u8 button;
    };

    std::unordered_map<std::string, ActionState> actions_;
    std::vector<KeyBinding> key_bindings_;
    std::vector<MouseBinding> mouse_bindings_;
    Vec2 mouse_pos_;
    Vec2 mouse_delta_;
    f32 wheel_x_ = 0;
    f32 wheel_y_ = 0;
};

} // namespace wander
