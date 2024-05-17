#pragma once

#include "jgmanager.h"

void audio_set_funcs();
void audio_init(JGManager *jgmgr);
void audio_deinit();
void audio_queue(size_t in_size);
void audio_pause();
void audio_unpause();
void audio_set_speed(int speed);
