#ifndef _INPUT_H_
#define _INPUT_H_

#define NUMGAMEPADS 2

#include <SDL.h>
#include "core/api/NstApiInput.hpp"

using namespace Nes::Api;

void input_init();
void input_process(Input::Controllers *controllers, SDL_Event event);

#endif
