#include "wander/core/log.h"
#include <cstdio>
#include <cstdarg>
#include <ctime>

namespace wander {

static LogLevel s_min_level = LogLevel::Debug;

void log_init(LogLevel min_level) {
    s_min_level = min_level;
}

void log_shutdown() {
    // Reserved for file logging cleanup
}

void log_set_level(LogLevel level) {
    s_min_level = level;
}

void log_message(LogLevel level, const char* file, int line, const char* fmt, ...) {
    if (level < s_min_level) return;

    static const char* level_names[] = {"DEBUG", "INFO", "WARN", "ERROR"};
    static const char* level_colors[] = {"\033[36m", "\033[32m", "\033[33m", "\033[31m"};

    // Extract just the filename from path
    const char* filename = file;
    for (const char* p = file; *p; p++) {
        if (*p == '/' || *p == '\\') filename = p + 1;
    }

    // Timestamp
    time_t now = time(nullptr);
    struct tm* tm_info = localtime(&now);
    char time_buf[16];
    strftime(time_buf, sizeof(time_buf), "%H:%M:%S", tm_info);

    // Print header
    fprintf(stderr, "%s[%s %s] %s:%d: \033[0m",
        level_colors[static_cast<int>(level)],
        time_buf,
        level_names[static_cast<int>(level)],
        filename, line);

    // Print message
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);

    fprintf(stderr, "\n");
}

} // namespace wander
