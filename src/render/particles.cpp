#include "wander/render/particles.h"
#include "wander/core/math.h"
#include <cstdlib>
#include <algorithm>

namespace wander {

static f32 rand_range(f32 lo, f32 hi) {
    f32 t = static_cast<f32>(std::rand()) / static_cast<f32>(RAND_MAX);
    return lo + t * (hi - lo);
}

static Color lerp_color(Color a, Color b, f32 t) {
    return Color(
        static_cast<u8>(a.r + (b.r - a.r) * t),
        static_cast<u8>(a.g + (b.g - a.g) * t),
        static_cast<u8>(a.b + (b.b - a.b) * t),
        static_cast<u8>(a.a + (b.a - a.a) * t)
    );
}

void ParticleEmitter::init(const ParticleEmitterConfig& config) {
    config_ = config;
    particles_.reserve(config.max_particles);

    if (config.burst_count > 0) {
        burst(config.burst_count);
        active_ = false;  // One-shot
    }
}

void ParticleEmitter::update(f32 dt) {
    // Emit new particles
    if (active_ && config_.emit_rate > 0.0f) {
        emit_accumulator_ += dt;
        f32 interval = 1.0f / config_.emit_rate;
        while (emit_accumulator_ >= interval) {
            emit(1);
            emit_accumulator_ -= interval;
        }
    }

    // Update existing particles
    for (auto& p : particles_) {
        p.velocity.y += config_.gravity * dt;
        p.position += p.velocity * dt;
        p.rotation += p.angular_velocity * dt;
        p.life += dt;
    }

    // Remove dead particles
    particles_.erase(
        std::remove_if(particles_.begin(), particles_.end(),
            [](const Particle& p) { return p.life >= p.max_life; }),
        particles_.end()
    );
}

void ParticleEmitter::render() {
    for (auto& p : particles_) {
        f32 t = p.life / p.max_life;
        f32 size = lerp(p.size, p.size_end, t);
        Color color = lerp_color(p.color_start, p.color_end, t);

        if (config_.texture && config_.texture->handle) {
            Rect dst = {p.position.x - size * 0.5f, p.position.y - size * 0.5f, size, size};
            draw_texture_ex(*config_.texture, config_.tex_region, dst,
                           p.rotation, {size * 0.5f, size * 0.5f}, false, false, color);
        } else {
            draw_rect({p.position.x - size * 0.5f, p.position.y - size * 0.5f, size, size}, color);
        }
    }
}

void ParticleEmitter::render(Vec2 cam_offset, f32 cam_scale) {
    for (auto& p : particles_) {
        f32 t = p.life / p.max_life;
        f32 size = lerp(p.size, p.size_end, t) * cam_scale;
        Color color = lerp_color(p.color_start, p.color_end, t);
        Vec2 sp = (p.position + cam_offset) * cam_scale;

        if (config_.texture && config_.texture->handle) {
            Rect dst = {sp.x - size * 0.5f, sp.y - size * 0.5f, size, size};
            draw_texture_ex(*config_.texture, config_.tex_region, dst,
                           p.rotation, {size * 0.5f, size * 0.5f}, false, false, color);
        } else {
            draw_rect({sp.x - size * 0.5f, sp.y - size * 0.5f, size, size}, color);
        }
    }
}

void ParticleEmitter::burst(i32 count) {
    emit(count);
}

void ParticleEmitter::set_position(Vec2 pos) {
    config_.position = pos;
}

bool ParticleEmitter::is_alive() const {
    return active_ || !particles_.empty();
}

void ParticleEmitter::emit(i32 count) {
    for (i32 i = 0; i < count; i++) {
        if (static_cast<i32>(particles_.size()) >= config_.max_particles) break;

        Particle p;
        p.position = config_.position + Vec2{
            rand_range(-config_.emit_offset.x, config_.emit_offset.x),
            rand_range(-config_.emit_offset.y, config_.emit_offset.y)
        };
        p.velocity = {
            rand_range(config_.velocity_min.x, config_.velocity_max.x),
            rand_range(config_.velocity_min.y, config_.velocity_max.y)
        };
        p.life = 0.0f;
        p.max_life = rand_range(config_.life_min, config_.life_max);
        p.size = config_.size_start;
        p.size_end = config_.size_end;
        p.color_start = config_.color_start;
        p.color_end = config_.color_end;
        p.rotation = rand_range(0.0f, TAU);
        p.angular_velocity = rand_range(-3.0f, 3.0f);

        particles_.push_back(p);
    }
}

} // namespace wander
