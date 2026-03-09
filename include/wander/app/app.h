#pragma once

#include "wander/core/types.h"
#include <SDL3/SDL_events.h>

namespace wander {

// Game callback interface — each game implements these
struct GameCallbacks {
    void (*on_init)() = nullptr;
    void (*on_event)(SDL_Event* event) = nullptr;
    void (*on_update)(f32 dt) = nullptr;
    void (*on_render)() = nullptr;
    void (*on_shutdown)() = nullptr;
    void (*on_pause)() = nullptr;    // Mobile: app backgrounded
    void (*on_resume)() = nullptr;   // Mobile: app foregrounded
};

struct AppConfig {
    const char* title = "Wander Engine";
    i32 window_width = 800;
    i32 window_height = 600;
    bool fullscreen = false;
    bool vsync = true;
    bool resizable = true;
    f32 target_fps = 60.0f;
};

// Run the app — blocks until quit. Returns 0 on success.
int app_run(const AppConfig& config, const GameCallbacks& callbacks);

// Request quit
void app_quit();

// Frame timing
f32 app_fps();
f32 app_dt();
u64 app_frame_count();

// Window info
Vec2i app_window_size();

} // namespace wander
