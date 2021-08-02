/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2021 R. Danbrook
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

#include "nstcommon.h"
#include "config.h"
#include "audio.h"

#define BUFSIZE 16000

extern Emulator emulator;

static SDL_AudioSpec spec, obtained;
static SDL_AudioDeviceID dev;
static SDL_AudioCVT cvt;

static int16_t intbuf[BUFSIZE];
static int16_t extbuf[BUFSIZE];
static uint16_t bufstart = 0;
static uint16_t bufend = 0;
static uint16_t bufsamples = 0;

static uint16_t framerate, channels, bufsize;

static bool paused = false;

void audio_set_speed(int speed) {
	bufsize = (channels * (conf.audio_sample_rate / framerate)) / speed;
}

void audio_queue() {
	while ((bufsamples + bufsize) >= BUFSIZE) { SDL_Delay(1); }

	SDL_LockAudioDevice(dev);
	int numsamples = bufsize;

	if (bufsamples < bufsize * 3) {
		SDL_ConvertAudio(&cvt);
		numsamples += channels * 2;
	}

	for (int i = 0; i < numsamples; i++) {
		extbuf[bufend] = intbuf[i];
		bufend = (bufend + 1) % BUFSIZE;
		bufsamples++;
		if (bufsamples >= BUFSIZE - 1) { break; }
	}

	SDL_UnlockAudioDevice(dev);
}

static inline float audio_dequeue() {
	if (bufsamples == 0) { return 0; }
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
	if (dev) { SDL_CloseAudioDevice(dev); }
}

void audio_init_sdl() {
	int e = 1; // Check Endianness
	SDL_AudioFormat fmt = ((int)*((unsigned char *)&e) == 1) ? AUDIO_S16LSB : AUDIO_S16MSB;

	spec.freq = conf.audio_sample_rate;
	spec.format = fmt;
	spec.channels = channels;
	spec.silence = 0;
	spec.samples = 512;
	spec.userdata = 0;
	spec.callback = audio_cb;

	bufsize = channels * (conf.audio_sample_rate / framerate);
	bufend = bufstart = bufsamples = 0;

	dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (!dev) {
		fprintf(stderr, "Error opening audio device.\n");
	}
	else {
		fprintf(stderr, "Audio: SDL - %dHz, %d channel(s)\n", spec.freq, spec.channels);
	}

	SDL_BuildAudioCVT(&cvt, fmt, channels, conf.audio_sample_rate, fmt, channels, conf.audio_sample_rate + ((nst_pal() ? 50 : 60) * channels * 2));
	SDL_assert(cvt.needed);
	cvt.len = (bufsize + channels * 2) * sizeof(int16_t);
	cvt.buf = (Uint8*)intbuf;

	SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
}

void audio_init() {
	// Initialize audio device
	// Set the framerate based on the region. For PAL: (60 / 6) * 5 = 50
	framerate = nst_pal() ? (conf.timing_speed / 6) * 5 : conf.timing_speed;
	channels = conf.audio_stereo ? 2 : 1;
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

void audio_set_params(Sound::Output *soundoutput) {
	// Set audio parameters
	Sound sound(emulator);

	sound.SetSampleBits(16);
	sound.SetSampleRate(conf.audio_sample_rate);

	sound.SetSpeaker(conf.audio_stereo ? Sound::SPEAKER_STEREO : Sound::SPEAKER_MONO);
	sound.SetSpeed(Sound::DEFAULT_SPEED);

	audio_adj_volume();

	soundoutput->samples[0] = intbuf;
	soundoutput->length[0] = conf.audio_sample_rate / framerate;
	soundoutput->samples[1] = NULL;
	soundoutput->length[1] = 0;
}

void audio_adj_volume() {
	// Adjust the audio volume to the current settings
	Sound sound(emulator);
	sound.SetVolume(Sound::ALL_CHANNELS, conf.audio_volume);
	sound.SetVolume(Sound::CHANNEL_SQUARE1, conf.audio_vol_sq1);
	sound.SetVolume(Sound::CHANNEL_SQUARE2, conf.audio_vol_sq2);
	sound.SetVolume(Sound::CHANNEL_TRIANGLE, conf.audio_vol_tri);
	sound.SetVolume(Sound::CHANNEL_NOISE, conf.audio_vol_noise);
	sound.SetVolume(Sound::CHANNEL_DPCM, conf.audio_vol_dpcm);
	sound.SetVolume(Sound::CHANNEL_FDS, conf.audio_vol_fds);
	sound.SetVolume(Sound::CHANNEL_MMC5, conf.audio_vol_mmc5);
	sound.SetVolume(Sound::CHANNEL_VRC6, conf.audio_vol_vrc6);
	sound.SetVolume(Sound::CHANNEL_VRC7, conf.audio_vol_vrc7);
	sound.SetVolume(Sound::CHANNEL_N163, conf.audio_vol_n163);
	sound.SetVolume(Sound::CHANNEL_S5B, conf.audio_vol_s5b);

	if (conf.audio_volume == 0) { memset(intbuf, 0, sizeof(int16_t) * BUFSIZE); }
}
