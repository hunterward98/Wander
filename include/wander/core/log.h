#pragma once

#include "wander/core/types.h"

namespace wander {

enum class LogLevel : u8 {
    Debug = 0,
    Info = 1,
    Warn = 2,
    Error = 3
};

void log_init(LogLevel min_level = LogLevel::Debug);
void log_shutdown();
void log_set_level(LogLevel level);
void log_message(LogLevel level, const char* file, int line, const char* fmt, ...);

} // namespace wander

// Macros for convenience — stripped in release builds for Debug level
#ifdef WANDER_DEBUG
    #define LOG_DEBUG(fmt, ...) ::wander::log_message(::wander::LogLevel::Debug, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#else
    #define LOG_DEBUG(fmt, ...) ((void)0)
#endif

#define LOG_INFO(fmt, ...)  ::wander::log_message(::wander::LogLevel::Info,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...)  ::wander::log_message(::wander::LogLevel::Warn,  __FILE__, __LINE__, fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) ::wander::log_message(::wander::LogLevel::Error, __FILE__, __LINE__, fmt, ##__VA_ARGS__)
