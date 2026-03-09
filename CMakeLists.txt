set(ENGINE_SOURCES
    # Core
    src/core/log.cpp
    src/core/memory.cpp
    src/core/debug.cpp
    # App
    src/app/app.cpp
    # Render
    src/render/renderer.cpp
    src/render/sprite.cpp
    src/render/sprite_batch.cpp
    src/render/camera.cpp
    src/render/text.cpp
    src/render/bitmap_font.cpp
    src/render/tilemap.cpp
    src/render/particles.cpp
    # Audio
    src/audio/audio.cpp
    # Input
    src/input/input.cpp
    # ECS
    src/ecs/ecs.cpp
    # Physics
    src/physics/physics.cpp
    # UI
    src/ui/ui.cpp
    # Script
    src/script/script.cpp
    # Scene
    src/scene/scene.cpp
    # Save
    src/save/save.cpp
    # stb implementations (compiled once here)
    src/stb_impl.cpp
)

add_library(wander_engine STATIC ${ENGINE_SOURCES})

target_include_directories(wander_engine
    PUBLIC include
    PRIVATE src
)

target_link_libraries(wander_engine
    PUBLIC
        SDL3::SDL3-static
        lua54
        miniaudio
        stb
        nlohmann_json
)

# Precompiled header — all consumers reuse this
target_precompile_headers(wander_engine PUBLIC include/wander/wander.h)

# Platform-specific link libs
if(WANDER_PLATFORM_LINUX)
    target_link_libraries(wander_engine PUBLIC m dl pthread)
elseif(WANDER_PLATFORM_MACOS)
    find_library(COREAUDIO CoreAudio)
    find_library(AUDIOTOOLBOX AudioToolbox)
    target_link_libraries(wander_engine PUBLIC ${COREAUDIO} ${AUDIOTOOLBOX})
endif()
