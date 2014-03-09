/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
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
#include <stdlib.h>
#include <string.h>

#include "config.h"
#include "audio.h"

#ifndef MINGW
#include <ao/ao.h>

ao_device *device;
ao_sample_format format;
#endif

#define NUMBUFFERS 2

extern settings conf;
extern Emulator emulator;
extern bool nst_pal;
extern bool updateok;

SDL_AudioSpec spec, obtained;
SDL_AudioDeviceID dev;

int16_t audiobuf[96000];

int framerate, channels;

int nDSoundSegLen = 0;

static volatile int16_t *buffer[NUMBUFFERS];
static volatile int bufstat[NUMBUFFERS];
static int playbuf, writebuf;
static uint8_t *curpos;
static int bytes_left;

void audio_fill_buffer(int bufnum) {
	int bytes_to_fill, bufpos;
	uint8_t *bufptr;
	
	//printf("FB%d\n", bufnum);
	
	// figure out how much we need out of this buffer
	// vs how much we can get out of it
	if (bytes_left >= bufstat[bufnum]) {
		bytes_to_fill = bufstat[bufnum];
	}
	else {
		bytes_to_fill = bytes_left;
	}
	
	// copy from the buffer's current position
	bufptr = (uint8_t*)buffer[bufnum];
	bufpos = (nDSoundSegLen*2*sizeof(int16_t)) - bufstat[bufnum];
	bufptr += bufpos;
	memcpy(curpos, bufptr, bytes_to_fill);
	
	// reduce the counters
	curpos += bytes_to_fill;
	bufstat[bufnum] -= bytes_to_fill;
	bytes_left -= bytes_to_fill;
}

void audio_sdl_callback(void *userdata, Uint8 *stream, int len) {
	int temp;
	
	curpos = stream;
	bytes_left = len;
	
	// need more data?
	while (bytes_left > 0) {
		// does our current buffer have any samples?
		if (bufstat[playbuf] > 0) {
			audio_fill_buffer(playbuf);
		}
		else {
			// check if the next buffer would collide
			temp = playbuf + 1;
			if (temp >= NUMBUFFERS) {
				temp = 0;
			}
			
			// no collision, set it and continue looping
			if (temp != writebuf) {
				playbuf = temp;
			}
			else {
			  //printf("UF\n");
				// underflow!
				memset(curpos, 0, bytes_left);
				bytes_left = 0;
			}
		}
	}
}

void audio_set_samples(uint32_t samples_per_frame) {
	// Set the number of samples per frame
	nDSoundSegLen = samples_per_frame;
	
	for (int i = 0; i < NUMBUFFERS; i++) {
		if (buffer[i]) {
			free((void *)buffer[i]);
			buffer[i] = (volatile int16_t *)NULL;
		}
		
		buffer[i] = (volatile int16_t *)malloc(nDSoundSegLen * 2 * sizeof(uint16_t));
		
		if (!buffer[i]) {
			fprintf(stderr, "Couldn't alloc buffer for SDL audio!\n");
			exit(-1);
		}
		
		memset((void *)buffer[i], 0, nDSoundSegLen * 2 * sizeof(uint16_t));
		
		bufstat[i] = 0;
	}
	
	playbuf = 0;
	writebuf = 1;
}

void audio_update() {	
	audio_output_frame(nDSoundSegLen, (int16_t *)buffer[writebuf]);
		
	// You can speed it up by manipulating the following line
	bufstat[writebuf] = nDSoundSegLen * 2 * sizeof(uint16_t);
	//printf("len: %d\n", nDSoundSegLen);
	
	if (++writebuf >= NUMBUFFERS) {
		writebuf = 0;
	}
}

void audio_init() {
	// Initialize audio device
	
	// Set the framerate based on the region. For PAL: (60 / 6) * 5 = 50
	framerate = nst_pal ? (conf.timing_speed / 6) * 5 : conf.timing_speed;
	
	channels = conf.audio_stereo ? 2 : 1;
	
	memset(audiobuf, 0, sizeof(audiobuf));
	
	if (conf.audio_api == 0) { // SDL
		spec.freq = conf.audio_sample_rate;
		spec.format = AUDIO_S16SYS;
		spec.channels = channels;
		spec.silence = 0;
		spec.samples = conf.audio_sample_rate / framerate;
		spec.userdata = 0;
		spec.callback = audio_sdl_callback;
		
		dev = SDL_OpenAudioDevice(NULL, 0, &spec, &obtained, SDL_AUDIO_ALLOW_ANY_CHANGE);
		if (!dev) {
			fprintf(stderr, "Error opening audio device.\n");
		}
		else {
			fprintf(stderr, "Audio: SDL - %dHz %d-bit, %d channel(s)\n", spec.freq, 16, spec.channels);
		}
		
		SDL_PauseAudioDevice(dev, 1);  // Setting to 0 unpauses
	}
#ifndef MINGW
	else if (conf.audio_api == 1) { // libao
		ao_initialize();
		
		int default_driver = ao_default_driver_id();
		
		memset(&format, 0, sizeof(format));
		format.bits = 16;
		format.channels = channels;
		format.rate = conf.audio_sample_rate;
		format.byte_format = AO_FMT_NATIVE;
		
		device = ao_open_live(default_driver, &format, NULL);
		if (device == NULL) {
			fprintf(stderr, "Error opening audio device.\n");
		}
		else {
			fprintf(stderr, "Audio: libao - %dHz, %d-bit, %d channel(s)\n", format.rate, format.bits, format.channels);
		}
	}
#endif
}

void audio_deinit() {
	// Deinitialize audio
	
	if (conf.audio_api == 0) { // SDL
		SDL_CloseAudioDevice(dev);
	}
#ifndef MINGW
	else if (conf.audio_api == 1) { // libao
		ao_close(device);
		ao_shutdown();
	}
#endif
	
	for (int i = 0; i < NUMBUFFERS; i++) {
		if (buffer[i]) {
			free((void*)buffer[i]);
			buffer[i] = (volatile int16_t*)NULL;
		}
	}
	
	memset(audiobuf, 0, sizeof(audiobuf));
}

void audio_pause() {
	// Pause the SDL audio device
	if (conf.audio_api == 0) { // SDL
		SDL_PauseAudioDevice(dev, 1);
	}
}

void audio_unpause() {
	// Unpause the SDL audio device
	if (conf.audio_api == 0) { // SDL
		SDL_PauseAudioDevice(dev, 0);
	}
}

void audio_play(Sound::Output *soundoutput) {
#ifndef MINGW	
	soundoutput->samples[0] = &audiobuf[0];

	if (conf.audio_api == 1) { // libao
		int bufsize = 2 * channels * (conf.audio_sample_rate / framerate);
		ao_play(device, (char*)audiobuf, bufsize);
	}
#endif
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
	
	sound.SetSpeaker(conf.audio_stereo ? Sound::SPEAKER_STEREO : Sound::SPEAKER_MONO);
	sound.SetSpeed(Sound::DEFAULT_SPEED);
	
	soundoutput->samples[0] = audiobuf;
	soundoutput->length[0] = conf.audio_sample_rate / framerate;
	soundoutput->samples[1] = NULL;
	soundoutput->length[1] = 0;
}

void audio_output_frame(unsigned long numsamples, int16_t *out) {
	int16_t *pbufL = (int16_t *)audiobuf;
	int16_t *outbuf;

	outbuf = out;
	if (conf.audio_stereo) {
		for (int s = 0; s < numsamples; s++) {
			*out++ = *pbufL++;
			*out++ = *pbufL++;
		}
	}
	else {
		for (int s = 0; s < numsamples; s++) {
			*out++ = *pbufL;
			*out++ = *pbufL++;
		}
	}

	updateok = true;
}

// Timing Functions

void timing_check() {
	//SDL_LockAudio();
	while ((bufstat[writebuf] == 0) && (writebuf != playbuf)) {
		audio_update();
	}
	//SDL_UnlockAudio();
	//usleep(50);
}

void timing_set_default() {
	// Set the framerate to the default
	framerate = nst_pal ? (conf.timing_speed / 6) * 5 : conf.timing_speed;
	SDL_GL_SetSwapInterval(conf.timing_vsync);
}

void timing_set_altspeed() {
	// Set the framerate to the alternate speed
	framerate = conf.timing_altspeed;
	SDL_GL_SetSwapInterval(0);
}
