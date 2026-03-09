# Wander Engine

A lightweight 2D game engine written in C++17 with SDL3. Designed for pixel art games targeting desktop (Windows, macOS, Linux) and mobile (iOS, Android).

## Modules

| Module | Description |
|--------|-------------|
| `core/` | Types, Vec2/Mat3 math, logging, arena/pool allocators, debug overlay |
| `app/` | SDL3 window management, fixed-timestep game loop (60hz) |
| `render/` | Renderer, sprites, sprite batching, camera, TrueType + bitmap fonts, particles, tilemaps |
| `audio/` | BGM/SFX via miniaudio, crossfade, volume groups |
| `input/` | Action-mapped input (keyboard, mouse, gamepad, touch) |
| `ecs/` | Sparse set ECS -- `World::each<Components...>(lambda)` |
| `physics/` | Spatial hash, AABB/circle collision, raycasting, triggers, move-and-slide |
| `ui/` | Immediate-mode widgets (buttons, sliders, panels, tooltips) |
| `script/` | Lua 5.4 VM with C++ bindings, hot-reload |
| `scene/` | Stack-based scene manager with transitions |
| `save/` | Binary save/load with versioning |

## Usage

Include the single umbrella header and use the `wander` namespace:

```cpp
#include <wander/wander.h>
using namespace wander;

int main() {
    AppConfig config;
    config.title = "My Game";
    config.window_width = 800;
    config.window_height = 600;

    GameCallbacks callbacks;
    callbacks.on_init = []() { /* load resources */ };
    callbacks.on_update = [](f32 dt) { /* game logic */ };
    callbacks.on_render = []() { /* draw */ };
    callbacks.on_event = [](SDL_Event* e) { /* input */ };
    callbacks.on_shutdown = []() { /* cleanup */ };

    return app_run(config, callbacks);
}
```

Link against `wander_engine` in your CMakeLists.txt:

```cmake
add_executable(my_game src/main.cpp)
target_link_libraries(my_game PRIVATE wander_engine)
target_precompile_headers(my_game REUSE_FROM wander_engine)
```

## Dependencies

All built from source via the `third_party/` directory in the parent repo:

- **SDL3** -- Windowing, rendering, input
- **Lua 5.4** -- Scripting, hot-reload
- **miniaudio** -- Audio playback
- **stb** -- Image loading, font rasterization, rect packing
- **nlohmann/json** -- Configuration files

## Structure

```
include/wander/    -- Public headers (one per module)
  wander.h         -- Umbrella include
  core/            -- types.h, math.h, log.h, platform.h, memory.h, debug.h
  app/             -- app.h
  render/          -- renderer.h, sprite.h, sprite_batch.h, camera.h, text.h, bitmap_font.h, particles.h, tilemap.h
  audio/           -- audio.h
  input/           -- input.h
  ecs/             -- ecs.h
  physics/         -- physics.h
  ui/              -- ui.h
  script/          -- script.h
  scene/           -- scene.h
  save/            -- save.h
src/               -- Implementation files (mirrors include/ layout)
```
