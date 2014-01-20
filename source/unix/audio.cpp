/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2014 R. Danbrook
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

#include <ao/ao.h>

#include "config.h"
#include "audio.h"

extern settings conf;
extern Emulator emulator;

SDL_AudioSpec spec, obtained;
SDL_AudioDeviceID dev;

ao_device *device;
ao_sample_format format;
char *outputbuf;

static int16_t *audiobuf;
static uint32_t outputbufsize;

int framerate;

void audio_init() {
	// Initialize audio device
	
	framerate = conf.timing_speed;
	
	outputbufsize = 4800; // This is a magic number
	audiobuf = (int16_t *)malloc(outputbufsize);
	memset(audiobuf, 0, outputbufsize);
	
	if (conf.audio_api == 0) { // SDL
		spec.freq = conf.audio_sample_rate;
		spec.format = AUDIO_S16SYS;
		spec.channels = 1;
		spec.silence = 0;
		spec.samples = conf.audio_sample_rate / 60;
		spec.userdata = 0;
		spec.callback = audio_callback;
		
		dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
		
		SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
	}
	
	else if (conf.audio_api == 1) { // libao
		ao_initialize();
		
		int default_driver = ao_default_driver_id();
		
		memset(&format, 0, sizeof(format));
		format.bits = 16;
		format.channels = 1;
		format.rate = conf.audio_sample_rate;
		format.byte_format = AO_FMT_NATIVE;
		
		device = ao_open_live(default_driver, &format, NULL);
		if (device == NULL) {
			fprintf(stderr, "Error opening audio device.\n");
		}
	}
}

void audio_set_params(Sound::Output *soundoutput) {
	// Set audio parameters
	Sound sound(emulator);
	
	sound.SetSampleBits(16);
	sound.SetSampleRate(conf.audio_sample_rate);
	
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
	
	sound.SetSpeaker(Sound::SPEAKER_MONO);
	//sound.SetSpeaker(Sound::SPEAKER_STEREO);
	sound.SetSpeed(Sound::DEFAULT_SPEED);
	
	soundoutput->samples[0] = audiobuf;
	soundoutput->length[0] = conf.audio_sample_rate/60;
	soundoutput->samples[1] = NULL;
	soundoutput->length[1] = 0;
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
	// Audio callback for SDL
	int i;
	Uint8 *outputbuf = (Uint8*)audiobuf;
	
	SDL_memset(stream, 0, len);
	
	for (i = 0; i < len; i++) {
		stream[i] = outputbuf[i];
	}
}

void audio_play() {
	
	if (conf.audio_api == 1) { // libao
		ao_play(device, (char*)audiobuf, (conf.audio_sample_rate / framerate) * 2);
	}
}

void audio_unpause() {
	// Unpause the SDL audio device
	if (conf.audio_api == 0) { // SDL
		SDL_PauseAudioDevice(dev, 0);
	}
}

void audio_deinit() {
	// Deinitialize audio
	if (audiobuf) { free(audiobuf); }
	
	if (conf.audio_api == 0) { // SDL
		SDL_CloseAudioDevice(dev);
	}
	else if (conf.audio_api == 1) { // libao
		ao_close(device);
		ao_shutdown();
	}
}

// Timing functions
void timing_set_default() {
	// Set the framerate to the default
	framerate = conf.timing_speed;
	SDL_GL_SetSwapInterval(conf.timing_vsync);
}

void timing_set_altspeed() {
	// Set the framerate to the alternate speed
	framerate = conf.timing_altspeed;
	SDL_GL_SetSwapInterval(0);
}
