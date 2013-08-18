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
inputsettings *inputconf;

SDL_Joystick *joystick;
gamepad player[NUMGAMEPADS];

GKeyFile *inputfile;
static GKeyFileFlags flags;
static gsize length;

char inputconfpath[256];

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
	
	int i;
	
	switch(event.type) {
		// Process Keyboard Events
		case SDL_KEYUP:
		case SDL_KEYDOWN:
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
			for(i = 0; i < NUMGAMEPADS; i++) {
				// Press Keys
				if (keys[player[i].u]) { controllers->pad[i].buttons |= Input::Controllers::Pad::UP; }
				if (keys[player[i].d]) { controllers->pad[i].buttons |= Input::Controllers::Pad::DOWN; }
				if (keys[player[i].l]) { controllers->pad[i].buttons |= Input::Controllers::Pad::LEFT; }
				if (keys[player[i].r]) { controllers->pad[i].buttons |= Input::Controllers::Pad::RIGHT; }
				if (keys[player[i].select]) { controllers->pad[i].buttons |= Input::Controllers::Pad::SELECT; }
				if (keys[player[i].start]) { controllers->pad[i].buttons |= Input::Controllers::Pad::START; }
				if (keys[player[i].a]) { controllers->pad[i].buttons |= Input::Controllers::Pad::A; }
				if (keys[player[i].b]) { controllers->pad[i].buttons |= Input::Controllers::Pad::B; }
				
				// Release Keys
				if (!keys[player[i].u]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::UP; }
				if (!keys[player[i].d]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::DOWN; }
				if (!keys[player[i].l]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::LEFT; }
				if (!keys[player[i].r]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::RIGHT; }
				if (!keys[player[i].select]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::SELECT; }
				if (!keys[player[i].start]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::START; }
				if (!keys[player[i].a]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::A; }
				if (!keys[player[i].b]) { controllers->pad[i].buttons &= ~Input::Controllers::Pad::B; }
			}
			break;
		
		// Process Joystick Button Events
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
			printf("Joystick Button Event\n");
			break;
		
		// Process Joystick Hat Events
		case SDL_JOYHATMOTION:
			printf("Joystick Hat Event\n");
			break;
		
		// Process Joystick Axis Events
		case SDL_JOYAXISMOTION:
			printf("Joystick Axis Event\n");
			break;
			
		default: break;
	}
}

void input_read_config() {
	
	char *homedir;

	homedir = getenv("HOME");
	snprintf(inputconfpath, sizeof(inputconfpath), "%s/.nestopia/input.conf", homedir);
	
	inputfile = g_key_file_new();
	
	flags = G_KEY_FILE_KEEP_COMMENTS;
	
	// Set aside memory for the input settings
	inputconf = g_slice_new(inputsettings);
	
	// Read the input configuration file
	if (g_key_file_load_from_file(inputfile, inputconfpath, flags, NULL)) {
		// Player 1
		inputconf->kb_p1u = g_key_file_get_string(inputfile, "gamepad1", "kb_u", NULL);
		inputconf->kb_p1d = g_key_file_get_string(inputfile, "gamepad1", "kb_d", NULL);
		inputconf->kb_p1l = g_key_file_get_string(inputfile, "gamepad1", "kb_l", NULL);
		inputconf->kb_p1r = g_key_file_get_string(inputfile, "gamepad1", "kb_r", NULL);
		inputconf->kb_p1select = g_key_file_get_string(inputfile, "gamepad1", "kb_select", NULL);
		inputconf->kb_p1start = g_key_file_get_string(inputfile, "gamepad1", "kb_start", NULL);
		inputconf->kb_p1a = g_key_file_get_string(inputfile, "gamepad1", "kb_a", NULL);
		inputconf->kb_p1b = g_key_file_get_string(inputfile, "gamepad1", "kb_b", NULL);
		
		// Player 2
		inputconf->kb_p2u = g_key_file_get_string(inputfile, "gamepad2", "kb_u", NULL);
		inputconf->kb_p2d = g_key_file_get_string(inputfile, "gamepad2", "kb_d", NULL);
		inputconf->kb_p2l = g_key_file_get_string(inputfile, "gamepad2", "kb_l", NULL);
		inputconf->kb_p2r = g_key_file_get_string(inputfile, "gamepad2", "kb_r", NULL);
		inputconf->kb_p2select = g_key_file_get_string(inputfile, "gamepad2", "kb_select", NULL);
		inputconf->kb_p2start = g_key_file_get_string(inputfile, "gamepad2", "kb_start", NULL);
		inputconf->kb_p2a = g_key_file_get_string(inputfile, "gamepad2", "kb_a", NULL);
		inputconf->kb_p2b = g_key_file_get_string(inputfile, "gamepad2", "kb_b", NULL);	
	}
	
	// Map the input settings from the config file
	
	// Player 1
	player[0].u = SDL_GetScancodeFromName(inputconf->kb_p1u);
	player[0].d = SDL_GetScancodeFromName(inputconf->kb_p1d);
	player[0].l = SDL_GetScancodeFromName(inputconf->kb_p1l);
	player[0].r = SDL_GetScancodeFromName(inputconf->kb_p1r);
	player[0].select = SDL_GetScancodeFromName(inputconf->kb_p1select);
	player[0].start = SDL_GetScancodeFromName(inputconf->kb_p1start);
	player[0].a = SDL_GetScancodeFromName(inputconf->kb_p1a);
	player[0].b = SDL_GetScancodeFromName(inputconf->kb_p1b);
	
	// Player 2
	player[1].u = SDL_GetScancodeFromName(inputconf->kb_p2u);
	player[1].d = SDL_GetScancodeFromName(inputconf->kb_p2d);
	player[1].l = SDL_GetScancodeFromName(inputconf->kb_p2l);
	player[1].r = SDL_GetScancodeFromName(inputconf->kb_p2r);
	player[1].select = SDL_GetScancodeFromName(inputconf->kb_p2select);
	player[1].start = SDL_GetScancodeFromName(inputconf->kb_p2start);
	player[1].a = SDL_GetScancodeFromName(inputconf->kb_p2a);
	player[1].b = SDL_GetScancodeFromName(inputconf->kb_p2b);
	
	// Free the input memory slice and file
	g_slice_free(inputsettings, inputconf);
	g_key_file_free(inputfile);
}
