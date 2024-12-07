/*
Copyright (c) 2012-2024 R. Danbrook
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice, this
   list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

3. Neither the name of the copyright holder nor the names of its
   contributors may be used to endorse or promote products derived from
   this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
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
