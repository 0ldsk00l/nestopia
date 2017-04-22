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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "audio.h"

#ifndef _MINGW
#include <ao/ao.h>

static ao_device *aodevice;
static ao_sample_format format;
#endif

extern settings_t conf;
extern Emulator emulator;
extern bool nst_pal;
extern bool updateok;

static SDL_AudioSpec spec, obtained;
static SDL_AudioDeviceID dev;

static int16_t audiobuf[6400];

static int framerate, channels, bufsize;

static bool altspeed = false;
static bool paused = false;

void (*audio_output)();
void (*audio_deinit)();

void audio_output_sdl() {
	//SDL_QueueAudio(dev, (const void*)audiobuf, bufsize);
	// Clear the audio queue arbitrarily to avoid it backing up too far
	//if (SDL_GetQueuedAudioSize(dev) > (Uint32)(bufsize * 3)) { SDL_ClearQueuedAudio(dev); }
}

void audio_output_ao() {
#ifndef _MINGW
	ao_play(aodevice, (char*)audiobuf, bufsize);
#endif
}

void audio_deinit_sdl() {
	SDL_CloseAudioDevice(dev);
}

void audio_deinit_ao() {
#ifndef _MINGW
	ao_close(aodevice);
	ao_shutdown();
#endif
}

void audio_play() {
	if (paused) { updateok = true; return; }
	bufsize = 2 * channels * (conf.audio_sample_rate / framerate);
	audio_output();
	updateok = true;
}

void audio_cb_sdl(void *data, uint8_t *stream, int len) {
	uint8_t *soundbuf = (uint8_t*)audiobuf;
	
	for (int i = 0; i < len; i++) {
		stream[i] = soundbuf[i];
	}
}

void audio_init_sdl() {
	spec.freq = conf.audio_sample_rate;
	spec.format = AUDIO_S16SYS;
	spec.channels = channels;
	spec.silence = 0;
	spec.samples = (conf.audio_sample_rate / framerate);
	spec.userdata = 0;
	//spec.callback = NULL; // Use SDL_QueueAudio instead
	spec.callback = audio_cb_sdl;
	
	dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
	if (!dev) {
		fprintf(stderr, "Error opening audio device.\n");
	}
	else {
		fprintf(stderr, "Audio: SDL - %dHz %d-bit, %d channel(s)\n", spec.freq, 16, spec.channels);
	}
	
	SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
}

void audio_init_ao() {
#ifndef _MINGW
	ao_initialize();
	
	int default_driver = ao_default_driver_id();
	
	memset(&format, 0, sizeof(format));
	format.bits = 16;
	format.channels = channels;
	format.rate = conf.audio_sample_rate;
	format.byte_format = AO_FMT_NATIVE;
	
	aodevice = ao_open_live(default_driver, &format, NULL);
	if (aodevice == NULL) {
		fprintf(stderr, "Error opening audio device.\n");
		aodevice = ao_open_live(ao_driver_id("null"), &format, NULL);
	}
	else {
		fprintf(stderr, "Audio: libao - %dHz, %d-bit, %d channel(s)\n", format.rate, format.bits, format.channels);
	}
#endif
}

void audio_init() {
	// Initialize audio device
	
	// Set the framerate based on the region. For PAL: (60 / 6) * 5 = 50
	framerate = nst_pal ? (conf.timing_speed / 6) * 5 : conf.timing_speed;
	channels = conf.audio_stereo ? 2 : 1;
	memset(audiobuf, 0, sizeof(audiobuf));
	
	#ifdef _MINGW
	conf.audio_api = 0; // Set SDL audio for MinGW
	#endif
	
	if (conf.audio_api == 0) { // SDL
		audio_init_sdl();
		audio_output = &audio_output_sdl;
		audio_deinit = &audio_deinit_sdl;
	}
	else if (conf.audio_api == 1) { // libao
		audio_init_ao();
		audio_output = &audio_output_ao;
		audio_deinit = &audio_deinit_ao;
	}
	
	paused = false;
}

void audio_pause() {
	// Pause the SDL audio device
	if (conf.audio_api == 0) { // SDL
		SDL_PauseAudioDevice(dev, 1);
	}
	paused = true;
}

void audio_unpause() {
	// Unpause the SDL audio device
	if (conf.audio_api == 0) { // SDL
		SDL_PauseAudioDevice(dev, 0);
	}
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
	
	soundoutput->samples[0] = audiobuf;
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
	
	if (conf.audio_volume == 0) { memset(audiobuf, 0, sizeof(audiobuf)); }
}

// Timing Functions

bool timing_frameskip() {
	// Calculate whether to skip a frame or not
	
	if (conf.audio_api == 0) { // SDL
		// Wait until the audio is drained
		//while (SDL_GetQueuedAudioSize(dev) > (Uint32)bufsize) {
		//	if (conf.timing_limiter) { SDL_Delay(1); }
		//}
	}
	
	static int fskip;
	fskip = altspeed ? (fskip > 1 ? 0 : fskip + 1) : 0;
	return fskip;
}

void timing_set_default() {
	// Set the framerate to the default
	altspeed = false;
	framerate = nst_pal ? (conf.timing_speed / 6) * 5 : conf.timing_speed;
	//if (conf.audio_api == 0) { SDL_ClearQueuedAudio(dev); }
}

void timing_set_altspeed() {
	// Set the framerate to the alternate speed
	altspeed = true;
	framerate = conf.timing_altspeed;
}
