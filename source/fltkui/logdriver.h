#pragma once

#include <cstdarg>

class LogDriver { // The Log Driver's Waltz pleases girls completely
public:
    static void jg_log(int level, const char *fmt, ...);
};
