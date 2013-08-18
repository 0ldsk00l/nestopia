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
	
	// Hardcoded Input Definitions for non-game related input
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
}
