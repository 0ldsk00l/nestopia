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

#include <cstdio>

#include <SDL.h>

#include <samplerate.h>

#include <jg/jg.h>

#include "audiomanager.h"
#include "jgmanager.h"

namespace {

jg_audioinfo_t* audinfo{nullptr};

SDL_AudioSpec spec, obtained;
SDL_AudioDeviceID dev;

SRC_STATE *srcstate{nullptr};
SRC_DATA srcdata;
float *fltbuf_in;
float *fltbuf_out;

int16_t *buf_in;
int16_t *buf_out;

size_t bufstart{0};
size_t bufend{0};
size_t bufsamples{0};

size_t spf;
int ffspeed{1};

void audio_cb_sdl(void *data, uint8_t *stream, int len) {
    AudioManager *audiomgr = static_cast<AudioManager*>(data);
    int16_t *out = (int16_t*)stream;
    for (int i = 0; i < len / sizeof(int16_t); i++) {
        out[i] = audiomgr->dequeue();
    }
}

};

AudioManager::AudioManager(JGManager& jgm, SettingManager& setmgr)
        : jgm(jgm), setmgr(setmgr) {
    jgm.set_audio_cb(AudioManager::queue);

    // Initialize audio buffers
    buf_in = new int16_t[BUFSIZE];
    std::fill(buf_in, buf_in + BUFSIZE, 0);
    buf_out = new int16_t[BUFSIZE];
    std::fill(buf_out, buf_out + BUFSIZE, 0);

    fltbuf_in = new float[BUFSIZE];
    std::fill(fltbuf_in, fltbuf_in + BUFSIZE, 0);
    fltbuf_out = new float[BUFSIZE];
    std::fill(fltbuf_out, fltbuf_out + BUFSIZE, 0);

    bufend = bufstart = bufsamples = 0;

    audinfo = jgm.get_audioinfo();
    audinfo->buf = &buf_in[0];
    spf = audinfo->spf;

    // Resampler
    int err;
    srcstate = src_new(setmgr.get_setting("a_rsqual")->val, audinfo->channels, &err);
    srcdata.data_in = fltbuf_in;
    srcdata.data_out = fltbuf_out;
    srcdata.output_frames = audinfo->spf;
    srcdata.src_ratio = 1.0;

    // SDL Audio Output
    spec.freq = audinfo->rate;
    #if SDL_BYTEORDER == SDL_BIG_ENDIAN
    spec.format = AUDIO_S16MSB;
    #else
    spec.format = AUDIO_S16LSB;
    #endif
    spec.channels = audinfo->channels;
    spec.silence = 0;
    spec.samples = 512;
    spec.userdata = this;
    spec.callback = audio_cb_sdl;

    dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
    if (!dev) {
        fprintf(stderr, "Error opening audio device.\n");
    }

    SDL_PauseAudioDevice(dev, 0);  // Setting to 0 unpauses
}

AudioManager::~AudioManager() {
    if (dev) {
        SDL_CloseAudioDevice(dev);
    }

    if (srcstate) {
        srcstate = src_delete(srcstate);
    }

    if (buf_in) {
        delete[] buf_in;
    }

    if (buf_out) {
        delete[] buf_out;
    }

    if (fltbuf_in) {
        delete[] fltbuf_in;
    }

    if (fltbuf_out) {
        delete[] fltbuf_out;
    }
}

int16_t AudioManager::dequeue() {
    if (bufsamples == 0) {
        return 0;
    }

    int16_t sample = buf_out[bufstart];
    bufstart = (bufstart + 1) % BUFSIZE;
    bufsamples--;
    return sample;
}

void AudioManager::queue(size_t in_size) {
    spf = audinfo->spf;
    while ((bufsamples + spf) >= BUFSIZE) {
        SDL_Delay(1);
    }

    size_t numsamples = in_size / ffspeed;

    src_short_to_float_array(buf_in, fltbuf_in, numsamples);
    srcdata.input_frames = numsamples;
    srcdata.end_of_input = 0;

    SDL_LockAudioDevice(dev);

    size_t frames_queued = bufsamples / spf;

    if (frames_queued < 3) {
        size_t step = 3 - frames_queued;
        srcdata.output_frames = numsamples + step;
        srcdata.src_ratio = (audinfo->rate + (JGManager::get_frametime() * step)) /
                            (audinfo->rate * 1.0);
    }
    else {
        srcdata.output_frames = numsamples;
        srcdata.src_ratio = 1.0;
    }

    src_process(srcstate, &srcdata);

    for (int i = 0; i < srcdata.output_frames_gen; i++) {
        fltbuf_out[i] *= 32768;
        buf_out[bufend] = fltbuf_out[i] >= 32767.0 ? 32767 :
                          fltbuf_out[i] <= -32768.0 ? -32768 :
                          fltbuf_out[i];
        bufend = (bufend + 1) % BUFSIZE;
        bufsamples++;
        if (bufsamples >= BUFSIZE - 1) {
            break;
        }
    }

    SDL_UnlockAudioDevice(dev);
}

void AudioManager::rehash() {
    if (srcstate) {
        srcstate = src_delete(srcstate);
    }
    int err;
    srcstate = src_new(setmgr.get_setting("a_rsqual")->val, audinfo->channels, &err);
}

void AudioManager::set_speed(int speed) {
    spf = (audinfo->rate / jgm.get_frametime()) / speed;
    ffspeed = speed;
}

void AudioManager::pause() {
    SDL_PauseAudioDevice(dev, 1);
}

void AudioManager::unpause() {
    SDL_PauseAudioDevice(dev, 0);
}
