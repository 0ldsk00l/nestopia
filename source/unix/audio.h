#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <SDL.h>
#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiSound.hpp"

using namespace Nes::Api;

void audio_init();
void audio_set_params(Sound::Output *soundoutput);
void audio_unpause();
void audio_play();
void audio_callback(void *userdata, Uint8 *stream, int len);
void audio_deinit();

void timing_set_default();
void timing_set_altspeed();

#endif
