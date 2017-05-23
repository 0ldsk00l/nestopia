#ifndef _AUDIO_H_
#define _AUDIO_H_

#include <SDL.h>
#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiSound.hpp"

using namespace Nes::Api;

extern void (*audio_deinit)();

void audio_init();
void audio_play();
void audio_pause();
void audio_unpause();
void audio_set_params(Sound::Output *soundoutput);
void audio_adj_volume();

int timing_runframes();
void timing_set_ffspeed();
void timing_set_default();

#endif
