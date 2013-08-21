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
	
	int player;
	nesinput input;
	
	// Process non-game events
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
	
	// Match keyboard and joystick input
	switch(event.type) {
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			input = input_match_keyboard(event);
			break;
			
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
			input = input_match_joystick(event);
			break;
			
		default:
			input.nescode = 0x00;
			input.player = 0;
			input.pressed = 0;
			break;
	}
	
	input_inject(controllers, input);
}

void input_inject(Input::Controllers *controllers, nesinput input) {
	
	if (input.pressed) {
		controllers->pad[input.player].buttons |= input.nescode;
	}
	else {
		controllers->pad[input.player].buttons &= ~input.nescode;
	}
}

nesinput input_match_joystick(SDL_Event event) {
	// Match NES buttons to joystick input
	int i;
	
	nesinput input;
	
	input.nescode = 0x00;
	input.player = 0;
	input.pressed = 0;

	return input;
}

nesinput input_match_keyboard(SDL_Event event) {
	// Match NES buttons to keyboard buttons
	int i;
	
	nesinput input;
	
	input.nescode = 0x00;
	input.player = 0;
	input.pressed = 0;
	
	if (event.type == SDL_KEYDOWN) { input.pressed = 1; }
	
	for (i = 0; i < NUMGAMEPADS; i++) {
		if (player[i].u == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::UP;
			input.player = i;
		}
		if (player[i].d == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::DOWN;
			input.player = i;
		}
		if (player[i].l == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::LEFT;
			input.player = i;
		}
		if (player[i].r == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::RIGHT;
			input.player = i;
		}
		if (player[i].select == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::SELECT;
			input.player = i;
		}
		if (player[i].start == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::START;
			input.player = i;
		}
		if (player[i].a == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::A;
			input.player = i;
		}
		if (player[i].b == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::B;
			input.player = i;
		}
	}
	
	return input;
}

char* input_translate_event(SDL_Event event) {
	// Translate an SDL_Event to an inputcode
	static char inputcode[6];
	
	switch(event.type) {
		case SDL_JOYAXISMOTION:
			if (abs(event.jaxis.value) > DEADZONE) {
				sprintf(inputcode, "j%da%d%d", event.jaxis.which, event.jaxis.axis, input_checksign(event.jaxis.value));
			}
			break;
			
		case SDL_JOYHATMOTION:
			sprintf(inputcode, "j%dh%d%d", event.jhat.which, event.jhat.hat, event.jhat.value);
			break;
		
		case SDL_JOYBUTTONUP:	
		case SDL_JOYBUTTONDOWN:
			sprintf(inputcode, "j%db%x", event.jbutton.which, event.jbutton.button);
			break;
	}
	return inputcode;
}

SDL_Event input_translate_string(char *string) {
	// Translate an inputcode to an SDL_Event
	SDL_Event event;
	
	int type;
	int which;
	int axis;
	int value;
	
	if ((unsigned char)string[2] == 0x61) { // Axis
		which = string[1] - '0';
		axis = string[3] - '0';
		value = string[4] - '0';
		event.type = SDL_JOYAXISMOTION;
		event.jaxis.which = which;
		event.jaxis.axis = axis;
		event.jaxis.value = value;
	}
	else if ((unsigned char)string[2] == 0x62) { // Button
		which = string[1] - '0';
		value = string[3] - '0';
		event.type = SDL_JOYBUTTONDOWN;
		event.jbutton.which = which;
		event.jbutton.button = value;
		
	}
	else if ((unsigned char)string[2] == 0x68) { // Hat
		which = string[1] - '0';
		axis = string[3] - '0';
		value = string[4] - '0';
		event.type = SDL_JOYHATMOTION;
		event.jhat.which = which;
		event.jhat.hat = axis;
		event.jhat.value = value;
	}
	else {
		fprintf(stderr, "Malformed inputcode: %s\n", string);
	}
	
	return event;
}

int input_checksign(int axisvalue) {
	
	if (axisvalue < 0) { return 0; }
	else { return 1; }
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
		
		inputconf->js_p1u = g_key_file_get_string(inputfile, "gamepad1", "js_u", NULL);
		inputconf->js_p1d = g_key_file_get_string(inputfile, "gamepad1", "js_d", NULL);
		inputconf->js_p1l = g_key_file_get_string(inputfile, "gamepad1", "js_l", NULL);
		inputconf->js_p1r = g_key_file_get_string(inputfile, "gamepad1", "js_r", NULL);
		inputconf->js_p1select = g_key_file_get_string(inputfile, "gamepad1", "js_select", NULL);
		inputconf->js_p1start = g_key_file_get_string(inputfile, "gamepad1", "js_start", NULL);
		inputconf->js_p1a = g_key_file_get_string(inputfile, "gamepad1", "js_a", NULL);
		inputconf->js_p1b = g_key_file_get_string(inputfile, "gamepad1", "js_b", NULL);
		
		// Player 2
		inputconf->kb_p2u = g_key_file_get_string(inputfile, "gamepad2", "kb_u", NULL);
		inputconf->kb_p2d = g_key_file_get_string(inputfile, "gamepad2", "kb_d", NULL);
		inputconf->kb_p2l = g_key_file_get_string(inputfile, "gamepad2", "kb_l", NULL);
		inputconf->kb_p2r = g_key_file_get_string(inputfile, "gamepad2", "kb_r", NULL);
		inputconf->kb_p2select = g_key_file_get_string(inputfile, "gamepad2", "kb_select", NULL);
		inputconf->kb_p2start = g_key_file_get_string(inputfile, "gamepad2", "kb_start", NULL);
		inputconf->kb_p2a = g_key_file_get_string(inputfile, "gamepad2", "kb_a", NULL);
		inputconf->kb_p2b = g_key_file_get_string(inputfile, "gamepad2", "kb_b", NULL);
		
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
		
		player[0].ju = input_translate_string(inputconf->js_p1u);
		player[0].jd = input_translate_string(inputconf->js_p1d);
		player[0].jl = input_translate_string(inputconf->js_p1l);
		player[0].jr = input_translate_string(inputconf->js_p1r);
		player[0].jselect = input_translate_string(inputconf->js_p1select);
		player[0].jstart = input_translate_string(inputconf->js_p1start);
		player[0].ja = input_translate_string(inputconf->js_p1a);
		player[0].jb = input_translate_string(inputconf->js_p1b);
		
		// Player 2
		player[1].u = SDL_GetScancodeFromName(inputconf->kb_p2u);
		player[1].d = SDL_GetScancodeFromName(inputconf->kb_p2d);
		player[1].l = SDL_GetScancodeFromName(inputconf->kb_p2l);
		player[1].r = SDL_GetScancodeFromName(inputconf->kb_p2r);
		player[1].select = SDL_GetScancodeFromName(inputconf->kb_p2select);
		player[1].start = SDL_GetScancodeFromName(inputconf->kb_p2start);
		player[1].a = SDL_GetScancodeFromName(inputconf->kb_p2a);
		player[1].b = SDL_GetScancodeFromName(inputconf->kb_p2b);
	}
	else {
		printf("Failed to load input config file %s: Using defaults.\n", inputconfpath);
	}
	
	// Free the input memory slice and file
	g_slice_free(inputsettings, inputconf);
	g_key_file_free(inputfile);
}
