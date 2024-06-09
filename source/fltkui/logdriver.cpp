/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2024 R. Danbrook
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
 * MA 02110-1301, USA.
 *
 */

#include <iostream>

#include "videomanager.h"

#include "logdriver.h"

namespace {
    LogLevel minlevel{LogLevel::Info};
}

void LogDriver::log(LogLevel level, std::string text) {
    if (level < minlevel) {
        return;
    }

    if (level == LogLevel::OSD) {
        VideoRenderer::text_print(text.c_str(), 16, 212, 2, true);
    }
    else if (level == LogLevel::Info) {
        std::cout << text << std::endl;
    }
    else {
        std::cerr << text << std::endl;
    }
}

void LogDriver::jg_log(int level, const char *fmt, ...) {
    if (level < static_cast<int>(minlevel)) {
        return;
    }

    va_list va;
    char buffer[512];
    static const char *lcol[4] = {
        "\033[0;35m", "\033[0;36m", "\033[7;33m", "\033[1;7;31m"
    };

    va_start(va, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, va);
    va_end(va);

    FILE *fout = level == 1 ? stdout : stderr;

    if (level == JG_LOG_SCR) {
        VideoRenderer::text_print(buffer, 16, 212, 2, true);
        return;
    }

    fprintf(fout, "%s%s\033[0m", lcol[level], buffer);
    fflush(fout);

    if (level == JG_LOG_ERR) {
        VideoRenderer::text_print(buffer, 16, 212, 2, true);
    }
}

void LogDriver::set_level(int level) {
    minlevel = static_cast<LogLevel>(level);
}
