#pragma once

// Platform detection
#if defined(__APPLE__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IOS
        #define WANDER_PLATFORM_IOS 1
    #else
        #define WANDER_PLATFORM_MACOS 1
    #endif
#elif defined(_WIN32)
    #define WANDER_PLATFORM_WINDOWS 1
#elif defined(__ANDROID__)
    #define WANDER_PLATFORM_ANDROID 1
#elif defined(__linux__)
    #define WANDER_PLATFORM_LINUX 1
#endif

// Mobile platform flag
#if defined(WANDER_PLATFORM_IOS) || defined(WANDER_PLATFORM_ANDROID)
    #define WANDER_PLATFORM_MOBILE 1
#else
    #define WANDER_PLATFORM_DESKTOP 1
#endif

// Debug flag
#if !defined(NDEBUG)
    #define WANDER_DEBUG 1
#endif

namespace wander {

// Runtime platform query
enum class Platform {
    Windows,
    MacOS,
    Linux,
    iOS,
    Android
};

constexpr Platform current_platform() {
#if defined(WANDER_PLATFORM_WINDOWS)
    return Platform::Windows;
#elif defined(WANDER_PLATFORM_MACOS)
    return Platform::MacOS;
#elif defined(WANDER_PLATFORM_LINUX)
    return Platform::Linux;
#elif defined(WANDER_PLATFORM_IOS)
    return Platform::iOS;
#elif defined(WANDER_PLATFORM_ANDROID)
    return Platform::Android;
#endif
}

constexpr bool is_mobile() {
#if defined(WANDER_PLATFORM_MOBILE)
    return true;
#else
    return false;
#endif
}

// Get platform-appropriate save directory for a given app name
// Implemented in platform.cpp
const char* get_save_path(const char* app_name);
const char* get_asset_path();

} // namespace wander
