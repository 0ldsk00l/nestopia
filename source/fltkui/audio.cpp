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
#include <cstring>

#include <SDL.h>

#include <jg/jg.h>

#include "audio.h"
#include "jgmanager.h"

#define BUFSIZE 16384

static SDL_AudioSpec spec, obtained;
static SDL_AudioDeviceID dev;
static SDL_AudioCVT cvt;

static JGManager *jgm = nullptr;
static jg_audioinfo_t* audinfo = nullptr;

static int16_t intbuf[BUFSIZE];
static int16_t extbuf[BUFSIZE];
static uint16_t bufstart = 0;
static uint16_t bufend = 0;
static uint16_t bufsamples = 0;

static size_t bufsize;
static int ffspeed = 1;

static bool paused = false;

void audio_set_speed(int speed) {
	bufsize = (audinfo->rate / jgm->get_frametime()) / speed;
	ffspeed = speed;
}

void audio_queue(size_t in_size) {
	while ((bufsamples + bufsize) >= BUFSIZE) {
		SDL_Delay(1);
	}

	size_t numsamples = in_size / ffspeed;

	SDL_LockAudioDevice(dev);

	if (bufsamples < bufsize * 3) {
		SDL_ConvertAudio(&cvt);
		numsamples += 2;
	}

	for (int i = 0; i < numsamples; i++) {
		extbuf[bufend] = intbuf[i];
		bufend = (bufend + 1) % BUFSIZE;
		bufsamples++;
		if (bufsamples >= BUFSIZE - 1) {
			break;
		}
	}

	SDL_UnlockAudioDevice(dev);
}

static inline int16_t audio_dequeue() {
	if (bufsamples == 0) {
		return 0;
	}

	int16_t sample = extbuf[bufstart];
	bufstart = (bufstart + 1) % BUFSIZE;
	bufsamples--;
	return sample;
}

void audio_cb(void *data, uint8_t *stream, int len) {
	int16_t *out = (int16_t*)stream;
	for (int i = 0; i < len / sizeof(int16_t); i++) {
		out[i] = audio_dequeue();
	}
}

void audio_deinit() {
	if (dev) {
		SDL_CloseAudioDevice(dev);
	}
}

void audio_init_sdl() {
	int e = 1; // Check Endianness
	SDL_AudioFormat fmt = ((int)*((unsigned char *)&e) == 1) ? AUDIO_S16LSB : AUDIO_S16MSB;

	spec.freq = audinfo->rate;
	spec.format = fmt;
	spec.channels = 1;
	spec.silence = 0;
	spec.samples = 512;
	spec.userdata = 0;
	spec.callback = audio_cb;

	// FIXME 60 should be frametime
	bufsize = audinfo->rate / 60;
	bufend = bufstart = bufsamples = 0;

	dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (!dev) {
		fprintf(stderr, "Error opening audio device.\n");
	}
	else {
		fprintf(stderr, "Audio: SDL - %dHz, %d channel(s)\n", spec.freq, spec.channels);
	}

	// FIXME 60 should be frametime
	SDL_BuildAudioCVT(&cvt, fmt, 1, audinfo->rate, fmt, 1, audinfo->rate + (60 * 2));
	SDL_assert(cvt.needed);
	cvt.len = (bufsize + 2) * sizeof(int16_t);
	cvt.buf = (Uint8*)intbuf;

	SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
}

void audio_init(JGManager *jgmgr) {
	jgm = jgmgr;
	audinfo = jg_get_audioinfo();
	audinfo->buf = &intbuf[0];

	// Initialize audio device
	memset(intbuf, 0, sizeof(int16_t) * BUFSIZE);
	audio_init_sdl();
	paused = false;
}

void audio_pause() {
	// Pause the SDL audio device
	SDL_PauseAudioDevice(dev, 1);
	paused = true;
}

void audio_unpause() {
	// Unpause the SDL audio device
	SDL_PauseAudioDevice(dev, 0);
	paused = false;
}
