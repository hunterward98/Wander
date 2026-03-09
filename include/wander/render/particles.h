#pragma once

#include "wander/core/types.h"
#include "wander/render/renderer.h"
#include <vector>

namespace wander {

// Single particle
struct Particle {
    Vec2 position;
    Vec2 velocity;
    f32 life = 0.0f;
    f32 max_life = 1.0f;
    f32 size = 4.0f;
    f32 size_end = 0.0f;
    Color color_start = Color::white();
    Color color_end = Color::transparent();
    f32 rotation = 0.0f;
    f32 angular_velocity = 0.0f;
};

// Emitter configuration
struct ParticleEmitterConfig {
    Vec2 position;
    Vec2 emit_offset = {0, 0};       // Random offset range
    Vec2 velocity_min = {-50, -50};
    Vec2 velocity_max = {50, 50};
    f32 gravity = 0.0f;
    f32 life_min = 0.5f;
    f32 life_max = 1.5f;
    f32 size_start = 4.0f;
    f32 size_end = 0.0f;
    Color color_start = Color::white();
    Color color_end = Color::transparent();
    f32 emit_rate = 20.0f;          // Particles per second
    i32 burst_count = 0;            // If > 0, emit this many immediately then stop
    i32 max_particles = 200;
    bool looping = true;
    Texture* texture = nullptr;     // Optional texture (nullptr = filled rect)
    Recti tex_region = {};          // Region if using texture atlas
};

// Particle emitter
class ParticleEmitter {
public:
    void init(const ParticleEmitterConfig& config);
    void update(f32 dt);
    void render();
    void burst(i32 count);
    void set_position(Vec2 pos);
    bool is_alive() const;
    i32 particle_count() const { return static_cast<i32>(particles_.size()); }

private:
    void emit(i32 count);

    ParticleEmitterConfig config_;
    std::vector<Particle> particles_;
    f32 emit_accumulator_ = 0.0f;
    bool active_ = true;
};

} // namespace wander
