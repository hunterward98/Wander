#include "wander/app/app.h"
#include "wander/core/log.h"
#include <SDL3/SDL.h>

namespace wander {

static struct {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    bool running = false;
    f32 fps = 0.0f;
    f32 dt = 0.0f;
    u64 frame_count = 0;
    Vec2i window_size;
} s_app;

// Expose SDL renderer to the rendering module
SDL_Renderer* app_get_sdl_renderer() { return s_app.renderer; }
SDL_Window* app_get_sdl_window() { return s_app.window; }

int app_run(const AppConfig& config, const GameCallbacks& callbacks) {
    // Initialize SDL (video is required, audio/gamepad are optional)
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        LOG_ERROR("SDL_Init failed: %s", SDL_GetError());
        return 1;
    }
    // Try to init audio and gamepad, but don't fail if unavailable
    if (!SDL_InitSubSystem(SDL_INIT_AUDIO)) {
        LOG_WARN("Audio init failed (non-fatal): %s", SDL_GetError());
    }
    if (!SDL_InitSubSystem(SDL_INIT_GAMEPAD)) {
        LOG_WARN("Gamepad init failed (non-fatal): %s", SDL_GetError());
    }
    LOG_INFO("SDL initialized");

    // Create window
    u32 window_flags = 0;
    if (config.fullscreen) window_flags |= SDL_WINDOW_FULLSCREEN;
    if (config.resizable) window_flags |= SDL_WINDOW_RESIZABLE;

    s_app.window = SDL_CreateWindow(
        config.title,
        config.window_width,
        config.window_height,
        window_flags
    );
    if (!s_app.window) {
        LOG_ERROR("SDL_CreateWindow failed: %s", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    LOG_INFO("Window created: %dx%d", config.window_width, config.window_height);

    // Create renderer (SDL3 picks best backend: GPU or software)
    s_app.renderer = SDL_CreateRenderer(s_app.window, nullptr);
    if (!s_app.renderer) {
        LOG_ERROR("SDL_CreateRenderer failed: %s", SDL_GetError());
        SDL_DestroyWindow(s_app.window);
        SDL_Quit();
        return 1;
    }

    // Set logical presentation for pixel-perfect integer scaling (opt-in)
    if (config.use_logical_presentation) {
        i32 logical_w = config.logical_width > 0 ? config.logical_width : config.window_width;
        i32 logical_h = config.logical_height > 0 ? config.logical_height : config.window_height;
        if (!SDL_SetRenderLogicalPresentation(s_app.renderer,
                logical_w, logical_h,
                SDL_LOGICAL_PRESENTATION_INTEGER_SCALE)) {
            LOG_WARN("Failed to set logical presentation: %s", SDL_GetError());
        }
    }

    // Apply fixed render scale (e.g. 2x for pixel-doubled rendering)
    if (config.render_scale != 1.0f) {
        SDL_SetRenderScale(s_app.renderer, config.render_scale, config.render_scale);
    }

    // Set minimum window size if specified
    if (config.min_window_width > 0 && config.min_window_height > 0) {
        SDL_SetWindowMinimumSize(s_app.window, config.min_window_width, config.min_window_height);
    }

    LOG_INFO("Renderer created: %s", SDL_GetRendererName(s_app.renderer));

    s_app.window_size = {config.window_width, config.window_height};
    s_app.running = true;

    // Game init
    if (callbacks.on_init) callbacks.on_init();

    // Fixed timestep game loop
    const f32 fixed_dt = 1.0f / config.target_fps;
    u64 prev_ticks = SDL_GetPerformanceCounter();
    u64 freq = SDL_GetPerformanceFrequency();
    f32 accumulator = 0.0f;
    u64 fps_timer = prev_ticks;
    u32 fps_frames = 0;

    while (s_app.running) {
        // Time
        u64 now_ticks = SDL_GetPerformanceCounter();
        f32 frame_time = static_cast<f32>(now_ticks - prev_ticks) / static_cast<f32>(freq);
        prev_ticks = now_ticks;

        // Clamp large frame times (e.g. after breakpoint or window drag)
        if (frame_time > 0.25f) frame_time = 0.25f;
        accumulator += frame_time;

        // FPS counter
        fps_frames++;
        f32 fps_elapsed = static_cast<f32>(now_ticks - fps_timer) / static_cast<f32>(freq);
        if (fps_elapsed >= 1.0f) {
            s_app.fps = static_cast<f32>(fps_frames) / fps_elapsed;
            fps_frames = 0;
            fps_timer = now_ticks;
        }

        // Poll events
        bool got_resize = false;
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            // Convert mouse coordinates from window to logical space
            SDL_ConvertEventToRenderCoordinates(s_app.renderer, &event);

            // Forward to game first
            if (callbacks.on_event) callbacks.on_event(&event);

            switch (event.type) {
                case SDL_EVENT_QUIT:
                    s_app.running = false;
                    break;
                case SDL_EVENT_WINDOW_RESIZED:
                    s_app.window_size.x = event.window.data1;
                    s_app.window_size.y = event.window.data2;
                    got_resize = true;
                    break;
                default:
                    break;
            }
        }

        // Fixed timestep updates
        while (accumulator >= fixed_dt) {
            s_app.dt = fixed_dt;
            if (callbacks.on_update) callbacks.on_update(fixed_dt);
            accumulator -= fixed_dt;
            s_app.frame_count++;
        }

        // Render + frame pacing (skip entirely during resize to prevent backlog)
        if (!got_resize) {
            SDL_SetRenderDrawColor(s_app.renderer, 20, 20, 30, 255);
            SDL_RenderClear(s_app.renderer);

            if (callbacks.on_render) callbacks.on_render();

            SDL_RenderPresent(s_app.renderer);

            // Manual frame pacing
            u64 end_ticks = SDL_GetPerformanceCounter();
            f32 frame_ms = static_cast<f32>(end_ticks - now_ticks) / static_cast<f32>(freq) * 1000.0f;
            f32 target_ms = 1000.0f / config.target_fps;
            if (frame_ms < target_ms) {
                SDL_Delay(static_cast<u32>(target_ms - frame_ms));
            }
        }
    }

    // Shutdown
    if (callbacks.on_shutdown) callbacks.on_shutdown();

    SDL_DestroyRenderer(s_app.renderer);
    SDL_DestroyWindow(s_app.window);
    SDL_Quit();
    LOG_INFO("Wander shutdown complete");
    return 0;
}

void app_quit() {
    s_app.running = false;
}

f32 app_fps() { return s_app.fps; }
f32 app_dt() { return s_app.dt; }
u64 app_frame_count() { return s_app.frame_count; }
Vec2i app_window_size() { return s_app.window_size; }

} // namespace wander
