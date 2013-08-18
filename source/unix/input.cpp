/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2013 R. Danbrook
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

#include "main.h"
#include "input.h"
#include "config.h"

extern settings *conf;

SDL_Joystick *joystick;

void input_init() {
	
	printf("%i joystick(s) found:\n", SDL_NumJoysticks());

	int i;

	for (i = 0; i < SDL_NumJoysticks(); i++) {
		joystick = SDL_JoystickOpen(i);
		printf("%s\n", SDL_JoystickName(joystick));
	}
}

void input_process(Input::Controllers *controllers, SDL_Event event) {
	
	const Uint8 *keybuffer = SDL_GetKeyboardState(NULL);
	Uint8 *keys = (Uint8*)keybuffer;
	
	if (event.type == SDL_KEYUP || event.type == SDL_KEYDOWN) {
		// Hardcoded Input Definitions for non-game input
		
		if (keys[SDL_SCANCODE_F1]) { FlipFDSDisk(); }
		if (keys[SDL_SCANCODE_F2]) { NstSoftReset(); }
		//if (keys[SDL_SCANCODE_F3]) {  }
		//if (keys[SDL_SCANCODE_F4]) {  }
		if (keys[SDL_SCANCODE_F5]) { QuickSave(0); }
		if (keys[SDL_SCANCODE_F6]) { QuickSave(1); }
		if (keys[SDL_SCANCODE_F7]) { QuickLoad(0); }
		if (keys[SDL_SCANCODE_F8]) { QuickLoad(1); }
		//if (keys[SDL_SCANCODE_F9]) {  }
		//if (keys[SDL_SCANCODE_F10]) {  }
		//if (keys[SDL_SCANCODE_F11]) {  }
		//if (keys[SDL_SCANCODE_F12]) {  }
		
		// Escape exits when not in GUI mode
		if (keys[SDL_SCANCODE_ESCAPE]) {
			if (conf->misc_disable_gui) { NstScheduleQuit(); }
		}
		
		// Process Game Input
		if (keys[SDL_SCANCODE_UP]) { controllers->pad[0].buttons |= Input::Controllers::Pad::UP; }
		if (keys[SDL_SCANCODE_DOWN]) { controllers->pad[0].buttons |= Input::Controllers::Pad::DOWN; }
		if (keys[SDL_SCANCODE_LEFT]) { controllers->pad[0].buttons |= Input::Controllers::Pad::LEFT; }
		if (keys[SDL_SCANCODE_RIGHT]) { controllers->pad[0].buttons |= Input::Controllers::Pad::RIGHT; }
		if (keys[SDL_SCANCODE_RSHIFT]) { controllers->pad[0].buttons |= Input::Controllers::Pad::SELECT; }
		if (keys[SDL_SCANCODE_RCTRL]) { controllers->pad[0].buttons |= Input::Controllers::Pad::START; }
		if (keys[SDL_SCANCODE_Z]) { controllers->pad[0].buttons |= Input::Controllers::Pad::A; }
		if (keys[SDL_SCANCODE_A]) { controllers->pad[0].buttons |= Input::Controllers::Pad::B; }
		
		// Release Keys
		if (!keys[SDL_SCANCODE_UP]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::UP; }
		if (!keys[SDL_SCANCODE_DOWN]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::DOWN; }
		if (!keys[SDL_SCANCODE_LEFT]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::LEFT; }
		if (!keys[SDL_SCANCODE_RIGHT]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::RIGHT; }
		if (!keys[SDL_SCANCODE_RSHIFT]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::SELECT; }
		if (!keys[SDL_SCANCODE_RCTRL]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::START; }
		if (!keys[SDL_SCANCODE_Z]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::A; }
		if (!keys[SDL_SCANCODE_A]) { controllers->pad[0].buttons &= ~Input::Controllers::Pad::B; }
	}
}
