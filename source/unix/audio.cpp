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

#ifdef _JACK
#define JACK_CLIENT_NAME "nestopia"
#define JACK_RB_SIZE 6400

#include <jack/jack.h>
#include <jack/ringbuffer.h>

static jack_port_t *jack_output_port1, *jack_output_port2;
static jack_client_t *jack_client;
static jack_ringbuffer_t *jack_rb = NULL;
static bool jack_ready = false;
const size_t jack_sample_size = sizeof(jack_default_audio_sample_t);
#endif

extern settings_t conf;
extern Emulator emulator;
extern bool nst_pal;

static SDL_AudioSpec spec, obtained;
static SDL_AudioDeviceID dev;

static int16_t audiobuf[6400];

static int framerate, channels, bufsize;

static bool paused = false;

void (*audio_output)();
void (*audio_deinit)();

void audio_output_sdl() {
	while (SDL_GetQueuedAudioSize(dev) > (Uint32)bufsize) {
		if (conf.timing_limiter) { SDL_Delay(1); }
	}
	SDL_QueueAudio(dev, (const void*)audiobuf, bufsize);
	// Clear the audio queue arbitrarily to avoid it backing up too far
	if (SDL_GetQueuedAudioSize(dev) > (Uint32)(bufsize * 3)) { SDL_ClearQueuedAudio(dev); }
}

void audio_output_ao() {
#ifndef _MINGW
	ao_play(aodevice, (char*)audiobuf, bufsize);
#endif
}

void audio_deinit_sdl() {
	if (dev) { SDL_CloseAudioDevice(dev); }
}

void audio_deinit_ao() {
#ifndef _MINGW
	if (aodevice) { ao_close(aodevice); ao_shutdown(); }
#endif
}

void audio_play() {
	if (paused) { return; }
	bufsize = 2 * channels * (conf.audio_sample_rate / framerate);
	audio_output();
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
	spec.callback = NULL; // Use SDL_QueueAudio instead
	//spec.callback = audio_cb_sdl;
	
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

#ifdef _JACK
int audio_cb_jack(jack_nframes_t nframes, void *arg) {
	jack_default_audio_sample_t *out1, *out2;
	int i;

	if(!jack_ready)
		return 0;

	out1 = (jack_default_audio_sample_t*) jack_port_get_buffer(jack_output_port1, nframes);
	out2 = (jack_default_audio_sample_t*) jack_port_get_buffer(jack_output_port2, nframes);
	// it is safe to not check if we have enough data
	// because jack_ringbuffer_read does a noop if you ask for too much
	for(i=0; i<nframes; i++) {
		jack_ringbuffer_read(jack_rb,(char *) (out1+i),jack_sample_size);
		if(channels == 1)
			out2[i] = out1[i];
		else if(channels == 2)
			jack_ringbuffer_read(jack_rb,(char *) (out2+i),jack_sample_size);
	}
	return 0;
}

void audio_jack_shutdown(void *arg) {
	jack_ready = false;
}

void audio_deinit_jack() {
	if(jack_client != NULL)
		jack_deactivate(jack_client);
	if(jack_rb != NULL)
		jack_ringbuffer_free(jack_rb);
	jack_ready = false;
}

void audio_init_jack() {
	const char **ports;
	jack_options_t options = JackNullOption;
	jack_status_t status;

	jack_client = jack_client_open(JACK_CLIENT_NAME,options,&status,NULL);
	if(jack_client == NULL) {
		fprintf(stderr, "Audio: jack - jack_client_open() failed, status = 0x%2.0x\n", status);
		if(status & JackServerFailed)
			fprintf(stderr, "Audio: jack - Unable to connect to JACK server\n");
		return;
	}
	// JACK needs sample rate to match server sample rate
	conf.audio_sample_rate = (int) jack_get_sample_rate(jack_client);
	fprintf(stderr, "Audio: jack - Setting sample rate to %d\n", conf.audio_sample_rate);
	jack_set_process_callback(jack_client, audio_cb_jack,NULL);

	// set up ringbuffer for callback
	jack_rb = jack_ringbuffer_create(JACK_RB_SIZE * channels * jack_sample_size);
	memset(jack_rb->buf, 0, jack_rb->size);

	jack_set_process_callback(jack_client, audio_cb_jack, NULL);
	jack_on_shutdown(jack_client, audio_jack_shutdown, NULL);

	jack_output_port1 = jack_port_register(jack_client, "nes out left", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	jack_output_port2 = jack_port_register(jack_client, "nes out right", JACK_DEFAULT_AUDIO_TYPE, JackPortIsOutput, 0);
	if((jack_output_port1 == NULL)||(jack_output_port2 == NULL)) {
		fprintf(stderr, "Audio: jack - No new ports available\n");
		return;
	}
	jack_activate(jack_client);
	ports = jack_get_ports(jack_client, NULL, NULL, JackPortIsPhysical | JackPortIsInput);
	if(ports == NULL) {
		fprintf(stderr, "Audio: jack - No playback ports available\n");
		return;
	}
	jack_connect(jack_client, jack_port_name(jack_output_port1), ports[0]);
	jack_connect(jack_client, jack_port_name(jack_output_port2), ports[1]);
	jack_free(ports);
	jack_ready = true;
}

void audio_output_jack() {
	size_t i;
	jack_default_audio_sample_t sample;

	if(!jack_ready)
		return;

	if(bufsize > (jack_ringbuffer_write_space(jack_rb) / jack_sample_size)) {
		fprintf(stderr, "Audio: jack - ringbuffer full!\n");
		bufsize = jack_ringbuffer_write_space(jack_rb) / jack_sample_size;
	}

	for(i = 0; i < (bufsize/2); i++) {
		// convert the audio to 32 bit float to make JACK happy
		sample = (float) audiobuf[i];
		sample /= (float) 0x8000;
		jack_ringbuffer_write(jack_rb, (char *) &sample, jack_sample_size);
	}
}
#endif // _JACK

void audio_set_funcs() {
	if (conf.audio_api == 0) { // SDL
		audio_output = &audio_output_sdl;
		audio_deinit = &audio_deinit_sdl;
	}
	else if (conf.audio_api == 1) { // libao
		audio_output = &audio_output_ao;
		audio_deinit = &audio_deinit_ao;
	}
	#ifdef _JACK
	else if (conf.audio_api == 2) { //JACK
		audio_output = &audio_output_jack;
		audio_deinit = &audio_deinit_jack;
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
	
	audio_set_funcs();
	
	if (conf.audio_api == 0) { audio_init_sdl(); }
	else if (conf.audio_api == 1) { audio_init_ao(); }
	#ifdef _JACK
	else if (conf.audio_api == 2) { audio_init_jack(); }
	#endif
	
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
