/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2016 R. Danbrook
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

ao_device *aodevice;
ao_sample_format format;
#endif

extern settings_t conf;
extern Emulator emulator;
extern bool nst_pal;
extern bool updateok;

SDL_AudioSpec spec, obtained;
SDL_AudioDeviceID dev;

int16_t audiobuf[96000];

int framerate, channels, bufsize;

bool altspeed = false;
bool paused = false;

void audio_play() {
	
	if (paused) { updateok = true; return; }
	
	bufsize = 2 * channels * (conf.audio_sample_rate / framerate);
	
	if (conf.audio_api == 0) { // SDL
		#if SDL_VERSION_ATLEAST(2,0,4)
		SDL_QueueAudio(dev, (const void*)audiobuf, bufsize);
		// Clear the audio queue arbitrarily to avoid it backing up too far
		if (SDL_GetQueuedAudioSize(dev) > (Uint32)(bufsize * 3)) { SDL_ClearQueuedAudio(dev); }
		#endif
	}
#ifndef _MINGW
	else if (conf.audio_api == 1) { // libao
		ao_play(aodevice, (char*)audiobuf, bufsize);
	}
#endif
	updateok = true;
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
	
	#if SDL_VERSION_ATLEAST(2,0,4)
	#else // Force libao if SDL lib is not modern enough
	if (conf.audio_api == 0) {
		conf.audio_api = 1;
		fprintf(stderr, "Audio: Forcing libao\n");
	}
	#endif
	
	if (conf.audio_api == 0) { // SDL
		spec.freq = conf.audio_sample_rate;
		spec.format = AUDIO_S16SYS;
		spec.channels = channels;
		spec.silence = 0;
		spec.samples = 512;
		spec.userdata = 0;
		spec.callback = NULL; // Use SDL_QueueAudio instead
		
		dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
		if (!dev) {
			fprintf(stderr, "Error opening audio device.\n");
		}
		else {
			fprintf(stderr, "Audio: SDL - %dHz %d-bit, %d channel(s)\n", spec.freq, 16, spec.channels);
		}
		
		SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
	}
#ifndef _MINGW
	else if (conf.audio_api == 1) { // libao
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
	}
#endif
	paused = false;
}

void audio_deinit() {
	// Deinitialize audio
	
	if (conf.audio_api == 0) { // SDL
		SDL_CloseAudioDevice(dev);
	}
#ifndef _MINGW
	else if (conf.audio_api == 1) { // libao
		ao_close(aodevice);
		ao_shutdown();
	}
#endif
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
		#if SDL_VERSION_ATLEAST(2,0,4)
		while (SDL_GetQueuedAudioSize(dev) > (Uint32)bufsize) {
			if (conf.timing_limiter) { SDL_Delay(1); }
		}
		#endif
	}
	
	static int flipper = 1;
	
	if (altspeed) {
		if (flipper > 2) { flipper = 0; return false; }
		else { flipper++; return true; }
	}
	
	return false;
}

void timing_set_default() {
	// Set the framerate to the default
	altspeed = false;
	framerate = nst_pal ? (conf.timing_speed / 6) * 5 : conf.timing_speed;
	#if SDL_VERSION_ATLEAST(2,0,4)
	if (conf.audio_api == 0) { SDL_ClearQueuedAudio(dev); }
	#endif
}

void timing_set_altspeed() {
	// Set the framerate to the alternate speed
	altspeed = true;
	framerate = conf.timing_altspeed;
}
