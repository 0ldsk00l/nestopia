/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2017 R. Danbrook
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

#define IBUFSIZE 4800
#define EBUFSIZE 9600

extern Emulator emulator;

static SDL_AudioSpec spec, obtained;
static SDL_AudioDeviceID dev;

static int16_t intbuf[IBUFSIZE];

static int16_t extbuf[EBUFSIZE];
static uint16_t bufstart = 0;
static uint16_t bufend = 0;
static uint16_t bufsamples = 0;

static int framerate, channels, bufsize;

static bool paused = false;

void audio_queue() {
	while ((bufsamples + bufsize) >= EBUFSIZE) { SDL_Delay(1); }
	
	SDL_LockAudioDevice(dev);
	for (int i = 0; i < bufsize; i++) {
		extbuf[bufend] = intbuf[i];
		bufend = (bufend + 1) % EBUFSIZE;
		bufsamples++;
		if (bufsamples >= EBUFSIZE - 1) { break; }
	}
	SDL_UnlockAudioDevice(dev);
}

static int16_t audio_dequeue() {
	if (bufsamples == 0) { return 0; }
	int16_t sample = extbuf[bufstart];
	bufstart = (bufstart + 1) % EBUFSIZE;
	bufsamples--;
	return sample;
}

void audio_cb(void *data, uint8_t *stream, int len) {
	int16_t *out = (int16_t*)stream;
	for (int i = 0; i < len / 2; i++) {
		out[i] = audio_dequeue();
	}
}

void audio_deinit() {
	if (dev) { SDL_CloseAudioDevice(dev); }
}

void audio_init_sdl() {
	spec.freq = conf.audio_sample_rate;
	spec.format = AUDIO_S16SYS;
	spec.channels = 1;
	spec.silence = 0;
	spec.samples = 512;
	spec.userdata = 0;
	spec.callback = audio_cb;
	
	bufsize = 1 * (conf.audio_sample_rate / framerate);
	
	dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (!dev) {
		fprintf(stderr, "Error opening audio device.\n");
	}
	else {
		fprintf(stderr, "Audio: SDL - %dHz %d-bit, %d channel(s)\n", spec.freq, 16, spec.channels);
	}
	
	SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
}

void audio_init() {
	// Initialize audio device
	// Set the framerate based on the region. For PAL: (60 / 6) * 5 = 50
	framerate = nst_pal() ? (conf.timing_speed / 6) * 5 : conf.timing_speed;
	channels = conf.audio_stereo ? 2 : 1;
	memset(intbuf, 0, sizeof(intbuf));
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
	//sound.SetSampleRate(conf.audio_sample_rate);
	sound.SetSampleRate(48000);
	
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
	
	if (conf.audio_volume == 0) { memset(intbuf, 0, sizeof(intbuf)); }
}
