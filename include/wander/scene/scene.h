#pragma once

#include "wander/core/types.h"
#include <vector>
#include <memory>
#include <functional>
#include <SDL3/SDL_events.h>

namespace wander {

// Scene base class
class Scene {
public:
    virtual ~Scene() = default;
    virtual void on_enter() {}
    virtual void on_exit() {}
    virtual void on_event(SDL_Event* event) { (void)event; }
    virtual void on_update(f32 dt) { (void)dt; }
    virtual void on_render() {}
    virtual void on_pause() {}   // When another scene is pushed on top
    virtual void on_resume() {}  // When the scene above is popped
};

// Transition effects
enum class TransitionType : u8 {
    None,
    Fade,
    SlideLeft,
    SlideRight,
    SlideUp,
    SlideDown
};

struct Transition {
    TransitionType type = TransitionType::None;
    f32 duration = 0.3f;
};

// Scene manager — stack-based
class SceneManager {
public:
    void push(std::unique_ptr<Scene> scene, Transition transition = {});
    void pop(Transition transition = {});
    void replace(std::unique_ptr<Scene> scene, Transition transition = {});

    void on_event(SDL_Event* event);
    void update(f32 dt);
    void render();

    Scene* current() const;
    bool empty() const { return stack_.empty(); }
    i32 depth() const { return static_cast<i32>(stack_.size()); }

private:
    struct SceneEntry {
        std::unique_ptr<Scene> scene;
    };

    std::vector<SceneEntry> stack_;

    // Transition state
    bool transitioning_ = false;
    f32 transition_timer_ = 0.0f;
    Transition transition_;
    std::unique_ptr<Scene> incoming_scene_;
    enum class TransitionAction { Push, Pop, Replace } transition_action_;
};

} // namespace wander
