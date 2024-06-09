#pragma once

#include <cstdarg>
#include <string>

enum class LogLevel : int { Debug, Info, Warn, Error, OSD };

class LogDriver { // A log driver's waltz pleases girls completely
public:
    static void log(LogLevel level, std::string text);
    static void jg_log(int level, const char *fmt, ...);
    static void set_level(int level);
};
