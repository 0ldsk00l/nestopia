/*
Copyright (c) 2023-2025 Rupert Carmichael
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

#ifndef AFILT_H
#define AFILT_H

#include <math.h>
#ifndef M_PI
#define M_PI 3.1415926535897932384626433832795
#endif

enum {
    FILTER_FO_LPF,
    FILTER_FO_HPF
};

typedef struct _filter_t {
    // Coefficients
    float a0;
    float a1;
    float a2;
    float b1;
    float b2;

    // State
    float x_1;
    float x_2;
    float y_1;
    float y_2;
} filter_t;

filter_t afilt_init(unsigned ftype, int fc, int fs);
void afilt(filter_t *f, int16_t *buf, unsigned samps);

#ifdef AFILT_IMPL

static filter_t afilter[2];

static inline int16_t flt_to_int16(float samp) {
    samp *= 32768;
    if (samp > 32767.0)
        return 32767;
    else if (samp <= -32768.0)
        return -32768;
    return samp;
}

/* Filter algorithms translated to C from "Designing Audio Effect Plugins in
   C++: For AAX, AU, and VST3 with DSP Theory" by Will Pirkle, Page 182
*/
// Initialize a First Order High or Low Pass Filter
filter_t afilt_init(unsigned ftype, int fc, int fs) {
    filter_t ret;
    float theta = 2.0 * M_PI * fc / fs;
    float gamma = cos(theta) / (1.0 + sin(theta));

    if (ftype) { // High Pass filter
        ret.a0 = (1.0 + gamma) / 2.0;
        ret.a1 = -((1.0 + gamma) / 2.0);
    }
    else { // Low Pass filter
        ret.a0 = (1.0 - gamma) / 2.0;
        ret.a1 = (1.0 - gamma) / 2.0;
    }

    ret.a2 = 0.0;

    ret.b1 = -gamma;
    ret.b2 = 0.0;

    ret.x_1 = ret.x_2 = ret.y_1 = ret.y_2 = 0;

    return ret;
}

void afilt(filter_t *f, int16_t *buf, unsigned samps) {
    // Copy values to avoid dereferencing many times
    float a0 = f->a0;
    float a1 = f->a1;
    float a2 = f->a2;
    float b1 = f->b1;
    float b2 = f->b2;

    // Restore the state
    float x_1 = f->x_1;
    float x_2 = f->x_2;
    float y_1 = f->y_1;
    float y_2 = f->y_2;
    float y = 0;

    for (unsigned i = 0; i < samps; ++i) {
        // Calculate the final output value
        y = (a0 * (buf[i] / 32768.0)) +
            (a1 * x_1) +
            (a2 * x_2) -
            (b1 * y_1) -
            (b2 * y_2);

        // Update intermediate state
        x_2 = x_1;
        x_1 = (buf[i] / 32768.0);
        y_2 = y_1;
        y_1 = y;

        // Record final output value in buffer
        buf[i] = flt_to_int16(y);
    }

    // Save the state
    f->x_1 = x_1;
    f->x_2 = x_2;
    f->y_1 = y_1;
    f->y_2 = y_2;
}

#endif // AFILT_IMPL
#endif // AFILT_H
