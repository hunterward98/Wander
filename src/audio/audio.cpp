#include "wander/audio/audio.h"
#include "wander/core/log.h"
#include "wander/core/math.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"

namespace wander {

static constexpr u32 MAX_SOUNDS = 256;

struct LoadedSound {
    ma_sound sound;
    bool loaded = false;
};

static struct {
    ma_engine engine;
    bool initialized = false;

    LoadedSound sounds[MAX_SOUNDS];
    u32 next_id = 1;

    // Music state
    SoundID current_music = INVALID_SOUND;
    f32 music_fade_timer = 0.0f;
    f32 music_fade_duration = 0.0f;
    bool music_fading_out = false;
    SoundID pending_music = INVALID_SOUND;

    // Volume per group
    f32 volumes[static_cast<int>(SoundGroup::Count)] = {1.0f, 1.0f, 1.0f, 1.0f};
} s_audio;

static f32 effective_volume(SoundGroup group) {
    return s_audio.volumes[static_cast<int>(SoundGroup::Master)] *
           s_audio.volumes[static_cast<int>(group)];
}

bool audio_init() {
    ma_engine_config config = ma_engine_config_init();
    config.channels = 2;
    config.sampleRate = 44100;

    if (ma_engine_init(&config, &s_audio.engine) != MA_SUCCESS) {
        LOG_WARN("Audio engine init failed (non-fatal)");
        return false;
    }

    s_audio.initialized = true;
    LOG_INFO("Audio system initialized (miniaudio)");
    return true;
}

void audio_shutdown() {
    if (!s_audio.initialized) return;

    for (u32 i = 0; i < MAX_SOUNDS; i++) {
        if (s_audio.sounds[i].loaded) {
            ma_sound_uninit(&s_audio.sounds[i].sound);
            s_audio.sounds[i].loaded = false;
        }
    }

    ma_engine_uninit(&s_audio.engine);
    s_audio.initialized = false;
    LOG_INFO("Audio system shutdown");
}

void audio_update(f32 dt) {
    if (!s_audio.initialized) return;

    // Handle music crossfade
    if (s_audio.music_fading_out && s_audio.music_fade_duration > 0.0f) {
        s_audio.music_fade_timer += dt;
        f32 t = s_audio.music_fade_timer / s_audio.music_fade_duration;

        if (t >= 1.0f) {
            // Fade out complete — stop old music
            if (s_audio.current_music != INVALID_SOUND) {
                u32 idx = s_audio.current_music - 1;
                if (idx < MAX_SOUNDS && s_audio.sounds[idx].loaded) {
                    ma_sound_stop(&s_audio.sounds[idx].sound);
                }
            }
            s_audio.music_fading_out = false;

            // Start pending music
            if (s_audio.pending_music != INVALID_SOUND) {
                s_audio.current_music = s_audio.pending_music;
                s_audio.pending_music = INVALID_SOUND;
                u32 idx = s_audio.current_music - 1;
                if (idx < MAX_SOUNDS && s_audio.sounds[idx].loaded) {
                    ma_sound_set_volume(&s_audio.sounds[idx].sound,
                        effective_volume(SoundGroup::BGM));
                    ma_sound_set_looping(&s_audio.sounds[idx].sound, MA_TRUE);
                    ma_sound_seek_to_pcm_frame(&s_audio.sounds[idx].sound, 0);
                    ma_sound_start(&s_audio.sounds[idx].sound);
                }
            }
        } else {
            // Fade out in progress
            if (s_audio.current_music != INVALID_SOUND) {
                u32 idx = s_audio.current_music - 1;
                if (idx < MAX_SOUNDS && s_audio.sounds[idx].loaded) {
                    ma_sound_set_volume(&s_audio.sounds[idx].sound,
                        effective_volume(SoundGroup::BGM) * (1.0f - t));
                }
            }
        }
    }
}

SoundID audio_load(const char* path) {
    if (!s_audio.initialized) return INVALID_SOUND;

    u32 id = s_audio.next_id;
    if (id > MAX_SOUNDS) {
        LOG_ERROR("Audio: max sounds exceeded (%d)", MAX_SOUNDS);
        return INVALID_SOUND;
    }

    u32 idx = id - 1;
    if (ma_sound_init_from_file(&s_audio.engine, path, 0, nullptr, nullptr,
                                 &s_audio.sounds[idx].sound) != MA_SUCCESS) {
        LOG_ERROR("Audio: failed to load %s", path);
        return INVALID_SOUND;
    }

    s_audio.sounds[idx].loaded = true;
    s_audio.next_id++;

    LOG_DEBUG("Audio: loaded %s (id=%d)", path, id);
    return id;
}

void audio_unload(SoundID id) {
    if (id == INVALID_SOUND || !s_audio.initialized) return;
    u32 idx = id - 1;
    if (idx >= MAX_SOUNDS || !s_audio.sounds[idx].loaded) return;

    ma_sound_uninit(&s_audio.sounds[idx].sound);
    s_audio.sounds[idx].loaded = false;
}

void audio_play(SoundID id, SoundGroup group, f32 volume) {
    if (id == INVALID_SOUND || !s_audio.initialized) return;
    u32 idx = id - 1;
    if (idx >= MAX_SOUNDS || !s_audio.sounds[idx].loaded) return;

    ma_sound_set_volume(&s_audio.sounds[idx].sound, volume * effective_volume(group));
    ma_sound_set_looping(&s_audio.sounds[idx].sound, MA_FALSE);
    ma_sound_seek_to_pcm_frame(&s_audio.sounds[idx].sound, 0);
    ma_sound_start(&s_audio.sounds[idx].sound);
}

void audio_play_music(SoundID id, f32 fade_time) {
    if (!s_audio.initialized) return;

    if (s_audio.current_music != INVALID_SOUND && fade_time > 0.0f) {
        s_audio.pending_music = id;
        s_audio.music_fading_out = true;
        s_audio.music_fade_timer = 0.0f;
        s_audio.music_fade_duration = fade_time;
    } else {
        s_audio.current_music = id;
        u32 idx = id - 1;
        if (idx < MAX_SOUNDS && s_audio.sounds[idx].loaded) {
            ma_sound_set_volume(&s_audio.sounds[idx].sound,
                effective_volume(SoundGroup::BGM));
            ma_sound_set_looping(&s_audio.sounds[idx].sound, MA_TRUE);
            ma_sound_seek_to_pcm_frame(&s_audio.sounds[idx].sound, 0);
            ma_sound_start(&s_audio.sounds[idx].sound);
        }
    }
}

void audio_stop_music(f32 fade_time) {
    if (!s_audio.initialized || s_audio.current_music == INVALID_SOUND) return;

    if (fade_time > 0.0f) {
        s_audio.pending_music = INVALID_SOUND;
        s_audio.music_fading_out = true;
        s_audio.music_fade_timer = 0.0f;
        s_audio.music_fade_duration = fade_time;
    } else {
        u32 idx = s_audio.current_music - 1;
        if (idx < MAX_SOUNDS && s_audio.sounds[idx].loaded) {
            ma_sound_stop(&s_audio.sounds[idx].sound);
        }
        s_audio.current_music = INVALID_SOUND;
    }
}

void audio_pause_music() {
    if (!s_audio.initialized || s_audio.current_music == INVALID_SOUND) return;
    u32 idx = s_audio.current_music - 1;
    if (idx < MAX_SOUNDS && s_audio.sounds[idx].loaded) {
        ma_sound_stop(&s_audio.sounds[idx].sound);
    }
}

void audio_resume_music() {
    if (!s_audio.initialized || s_audio.current_music == INVALID_SOUND) return;
    u32 idx = s_audio.current_music - 1;
    if (idx < MAX_SOUNDS && s_audio.sounds[idx].loaded) {
        ma_sound_start(&s_audio.sounds[idx].sound);
    }
}

void audio_set_volume(SoundGroup group, f32 volume) {
    s_audio.volumes[static_cast<int>(group)] = clamp(volume, 0.0f, 1.0f);
}

f32 audio_get_volume(SoundGroup group) {
    return s_audio.volumes[static_cast<int>(group)];
}

void audio_set_master_volume(f32 volume) {
    audio_set_volume(SoundGroup::Master, volume);
    if (s_audio.initialized) {
        ma_engine_set_volume(&s_audio.engine, volume);
    }
}

f32 audio_get_master_volume() {
    return audio_get_volume(SoundGroup::Master);
}

} // namespace wander
