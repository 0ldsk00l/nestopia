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

#include "config.h"
#include "newaudio.h"

extern settings conf;
extern Emulator emulator;

SDL_AudioSpec spec;
SDL_AudioDeviceID dev;

static int16_t *audiobuf;
static volatile uint32_t audiopos;
static uint32_t audiolength;
static uint32_t outputbufsize;

void audio_init() {
	// Initialize audio device
	
	spec.freq = conf.audio_sample_rate;
	spec.format = AUDIO_S16SYS;
	spec.channels = 2;
	spec.silence = 0;
	spec.samples = 512;
	//spec.padding = 0;
	//spec.size = 0;
	spec.userdata = 0;
	spec.callback = audio_callback;
	
	outputbufsize = 12800; // This is a magic number
	
	audiolength = conf.audio_sample_rate / 60;
	audiopos = 0;
	
	audiobuf = (int16_t *)malloc(outputbufsize);
	
	memset(audiobuf, 0, outputbufsize);
	
	dev = SDL_OpenAudioDevice(NULL, 0, &spec, NULL, SDL_AUDIO_ALLOW_ANY_CHANGE);
	
	SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
}

void audio_set_params(Sound::Output *soundoutput) {
	
	Sound sound(emulator);
	
	sound.SetSampleBits(16);
	sound.SetSampleRate(conf.audio_sample_rate);
	sound.SetVolume(Sound::ALL_CHANNELS, conf.audio_volume);
	
	//sound.SetSpeaker(Sound::SPEAKER_MONO);
	sound.SetSpeaker(Sound::SPEAKER_STEREO);
	sound.SetSpeed(Sound::DEFAULT_SPEED);
	
	soundoutput->samples[0] = audiobuf;
	soundoutput->length[0] = conf.audio_sample_rate/60;
	//soundoutput->length[0] = 735;
	soundoutput->samples[1] = NULL;
	soundoutput->length[1] = 0;
}

void audio_callback(void *userdata, Uint8 *stream, int len) {
	
	int i;
	Uint8 *outputbuf = (Uint8*)audiobuf; // Is this why it's scratchy?
	
	SDL_memset(stream, 0, len);

	for (i = 0; i < len; i++) {
		if (audiopos >= (outputbufsize / 2)) { audiopos = 0; }
		stream[i] = outputbuf[audiopos++];
	}
	
	//SDL_MixAudioFormat(stream, outputbuf, AUDIO_S16SYS, len, SDL_MIX_MAXVOLUME);
}

void audio_play() {
	SDL_PauseAudioDevice(dev, 0);
}

void audio_deinit() {
    SDL_CloseAudioDevice(dev);
    if (audiobuf) { free(audiobuf); }
}
