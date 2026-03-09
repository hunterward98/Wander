#pragma once

#include "wander/core/types.h"

namespace wander {

// Sound handle
using SoundID = u32;
constexpr SoundID INVALID_SOUND = 0;

// Sound groups for volume control
enum class SoundGroup : u8 {
    Master = 0,
    BGM,
    SFX,
    UI,
    Count
};

// Lifecycle
bool audio_init();
void audio_shutdown();
void audio_update(f32 dt);

// Load/unload sounds (WAV, OGG, MP3, FLAC via miniaudio)
SoundID audio_load(const char* path);
void audio_unload(SoundID id);

// Play a sound effect (fire-and-forget)
void audio_play(SoundID id, SoundGroup group = SoundGroup::SFX, f32 volume = 1.0f);

// Background music (with crossfade)
void audio_play_music(SoundID id, f32 fade_time = 0.5f);
void audio_stop_music(f32 fade_time = 0.5f);
void audio_pause_music();
void audio_resume_music();

// Volume control per group (0.0 - 1.0)
void audio_set_volume(SoundGroup group, f32 volume);
f32  audio_get_volume(SoundGroup group);
void audio_set_master_volume(f32 volume);
f32  audio_get_master_volume();

} // namespace wander
