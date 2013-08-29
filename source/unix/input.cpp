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
#include "config.h"
#include "video.h"
#include "input.h"

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

void input_deinit() {
	SDL_JoystickClose(joystick);
}

void input_process(Input::Controllers *controllers, SDL_Event event) {
	
	const Uint8 *keybuffer = SDL_GetKeyboardState(NULL);
	Uint8 *keys = (Uint8*)keybuffer;
	
	int player;
	
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
	
	// F toggles fullscreen
	if (keys[SDL_SCANCODE_F]) { video_toggle_fullscreen(); }
	
	// Match keyboard and joystick input
	switch(event.type) {
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			input_match_keyboard(controllers, event);
			break;
			
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
			input_match_joystick(controllers, event);
			break;
			
		default: break;
	}
}

void input_inject(Input::Controllers *controllers, nesinput input) {
	// Insert the input signal into the NES
	if (input.pressed) {
		controllers->pad[input.player].buttons |= input.nescode;
	}
	else {
		controllers->pad[input.player].buttons &= ~input.nescode;
	}
}

void input_match_joystick(Input::Controllers *controllers, SDL_Event event) {
	// Match NES buttons to joystick input
	int j;
	
	nesinput input, reverseinput;
	
	input.nescode = 0x00;
	input.player = 0;
	input.pressed = 0;
	
	// This is for releasing opposing directions
	reverseinput.nescode = 0x00;
	reverseinput.player = 0;
	reverseinput.pressed = 0;
	
	SDL_Event buttons[16] = {
		player[0].ju, player[0].jd, player[0].jl, player[0].jr,
		player[0].jselect, player[0].jstart, player[0].ja, player[0].jb,
		
		player[1].ju, player[1].jd, player[1].jl, player[1].jr,
		player[1].jselect, player[1].jstart, player[1].ja, player[1].jb
	};
	
	static unsigned char nescodes[16] = {
		Input::Controllers::Pad::UP,
		Input::Controllers::Pad::DOWN,
		Input::Controllers::Pad::LEFT,
		Input::Controllers::Pad::RIGHT,
		Input::Controllers::Pad::SELECT,
		Input::Controllers::Pad::START,
		Input::Controllers::Pad::A,
		Input::Controllers::Pad::B,
		Input::Controllers::Pad::UP,
		Input::Controllers::Pad::DOWN,
		Input::Controllers::Pad::LEFT,
		Input::Controllers::Pad::RIGHT,
		Input::Controllers::Pad::SELECT,
		Input::Controllers::Pad::START,
		Input::Controllers::Pad::A,
		Input::Controllers::Pad::B
	};
	
	switch(event.type) {
		// Handle button input
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
			for (j = 0; j < 16; j++) {
				if (buttons[j].jbutton.button == event.jbutton.button
					&& buttons[j].jbutton.which == event.jbutton.which) {
					input.nescode = nescodes[j];
					if (j >= 8) { input.player = 1; }
				}
			}
			input.pressed = event.jbutton.state;
			break;
		
		// Handling hat input can be a lot of fun if you like pain
		case SDL_JOYHATMOTION:
			unsigned char hu, hd, hl, hr;
			hu = hd = hl = hr = 0;
			
			// Start a loop to check if input matches
			for (j = 0; j < 16; j++) {
				
				// Read value of each hat direction on current hat
				if (buttons[j].type == event.type
					&& buttons[j].jhat.which == event.jhat.which
					&& buttons[j].jhat.hat == event.jhat.hat) {
					if (j >= 8) { input.player = reverseinput.player = 1; }

					// Find the values at each hat position on the current hat
					if (buttons[j].jhat.value == SDL_HAT_UP) { hu = nescodes[j]; }
					else if (buttons[j].jhat.value == SDL_HAT_DOWN) { hd = nescodes[j]; }
					else if (buttons[j].jhat.value == SDL_HAT_LEFT) { hl = nescodes[j]; }
					else if (buttons[j].jhat.value == SDL_HAT_RIGHT) { hr = nescodes[j]; }
					
					// Make sure opposing hat positions are turned off
					switch(event.jhat.value) {
						case SDL_HAT_UP:
							input.pressed = 1;
							input.nescode |= hu;
							reverseinput.nescode |= hd |= hl |= hr;
							break;
						case SDL_HAT_LEFTUP:
							input.pressed = 1;
							input.nescode |= hu |= hl;
							reverseinput.nescode |= hd |= hr;
							break;
						case SDL_HAT_RIGHTUP:
							input.pressed = 1;
							input.nescode |= hu |= hr;
							reverseinput.nescode |= hd |= hl;
							break;
						case SDL_HAT_DOWN:
							input.pressed = 1;
							input.nescode |= hd;
							reverseinput.nescode |= hu |= hl |= hr;
							break;
						case SDL_HAT_LEFTDOWN:
							input.pressed = 1;
							input.nescode |= hd |= hl;
							reverseinput.nescode |= hu |= hr;
							break;
						case SDL_HAT_RIGHTDOWN:
							input.pressed = 1;
							input.nescode |= hd |= hr;
							reverseinput.nescode |= hu |= hl;
							break;
						case SDL_HAT_LEFT:
							input.pressed = 1;
							input.nescode |= hl;
							reverseinput.nescode |= hr |= hu |= hd;
							break;
						case SDL_HAT_RIGHT:
							input.pressed = 1;
							input.nescode |= hr;
							reverseinput.nescode |= hl |= hu |= hd;
							break;
						default:
							input.nescode |= hu |= hd |= hl |= hr;
							break;
					}
				}
			}
			break;

		// Handle axis input
		case SDL_JOYAXISMOTION:
			for (j = 0; j < 16; j++) {
				
				int nvalue = input_checksign(event.jaxis.value);

				if (buttons[j].jaxis.axis == event.jaxis.axis
					&& buttons[j].jaxis.which == event.jaxis.which
					&& buttons[j].jaxis.type == event.jaxis.type
					&& buttons[j].jaxis.value == nvalue) {

					if (j >= 8) { input.player = reverseinput.player = 1; }
					
					input.nescode = nescodes[j];
				}

				if (buttons[j].jaxis.axis == event.jaxis.axis
					&& buttons[j].jaxis.which == event.jaxis.which
					&& buttons[j].jaxis.type == event.jaxis.type
					&& buttons[j].jaxis.value == !nvalue) {
					
					reverseinput.nescode = nescodes[j];
				}

				if (abs(event.jaxis.value) > DEADZONE) { input.pressed = 1; }
			}
			break;
			
		default: break;
	}
	
	input_inject(controllers, reverseinput);
	input_inject(controllers, input);
}

void input_match_keyboard(Input::Controllers *controllers, SDL_Event event) {
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
		else if (player[i].d == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::DOWN;
			input.player = i;
		}
		else if (player[i].l == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::LEFT;
			input.player = i;
		}
		else if (player[i].r == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::RIGHT;
			input.player = i;
		}
		else if (player[i].select == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::SELECT;
			input.player = i;
		}
		else if (player[i].start == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::START;
			input.player = i;
		}
		else if (player[i].a == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::A;
			input.player = i;
		}
		else if (player[i].b == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::B;
			input.player = i;
		}
	}
	
	input_inject(controllers, input);
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

	int nvalue = abs(axisvalue) < DEADZONE;
	
	if (axisvalue <= 0) { return 0; }
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

		inputconf->js_p2u = g_key_file_get_string(inputfile, "gamepad2", "js_u", NULL);
		inputconf->js_p2d = g_key_file_get_string(inputfile, "gamepad2", "js_d", NULL);
		inputconf->js_p2l = g_key_file_get_string(inputfile, "gamepad2", "js_l", NULL);
		inputconf->js_p2r = g_key_file_get_string(inputfile, "gamepad2", "js_r", NULL);
		inputconf->js_p2select = g_key_file_get_string(inputfile, "gamepad2", "js_select", NULL);
		inputconf->js_p2start = g_key_file_get_string(inputfile, "gamepad2", "js_start", NULL);
		inputconf->js_p2a = g_key_file_get_string(inputfile, "gamepad2", "js_a", NULL);
		inputconf->js_p2b = g_key_file_get_string(inputfile, "gamepad2", "js_b", NULL);
		
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
		
		player[1].ju = input_translate_string(inputconf->js_p2u);
		player[1].jd = input_translate_string(inputconf->js_p2d);
		player[1].jl = input_translate_string(inputconf->js_p2l);
		player[1].jr = input_translate_string(inputconf->js_p2r);
		player[1].jselect = input_translate_string(inputconf->js_p2select);
		player[1].jstart = input_translate_string(inputconf->js_p2start);
		player[1].ja = input_translate_string(inputconf->js_p2a);
		player[1].jb = input_translate_string(inputconf->js_p2b);
	}
	else {
		printf("Failed to load input config file %s: Using defaults.\n", inputconfpath);
		input_set_default();
	}
}

void input_write_config() {
	// Write out the input configuration file
	g_key_file_set_string(inputfile, "gamepad1", "kb_u", SDL_GetScancodeName(player[0].u));
	g_key_file_set_string(inputfile, "gamepad1", "kb_d", SDL_GetScancodeName(player[0].d));
	g_key_file_set_string(inputfile, "gamepad1", "kb_l", SDL_GetScancodeName(player[0].l));
	g_key_file_set_string(inputfile, "gamepad1", "kb_r", SDL_GetScancodeName(player[0].r));
	g_key_file_set_string(inputfile, "gamepad1", "kb_select", SDL_GetScancodeName(player[0].select));
	g_key_file_set_string(inputfile, "gamepad1", "kb_start", SDL_GetScancodeName(player[0].start));
	g_key_file_set_string(inputfile, "gamepad1", "kb_a", SDL_GetScancodeName(player[0].a));
	g_key_file_set_string(inputfile, "gamepad1", "kb_b", SDL_GetScancodeName(player[0].b));

	g_key_file_set_string(inputfile, "gamepad1", "js_u", input_translate_event(player[0].ju));
	g_key_file_set_string(inputfile, "gamepad1", "js_d", input_translate_event(player[0].jd));
	g_key_file_set_string(inputfile, "gamepad1", "js_l", input_translate_event(player[0].jl));
	g_key_file_set_string(inputfile, "gamepad1", "js_r", input_translate_event(player[0].jr));
	g_key_file_set_string(inputfile, "gamepad1", "js_select", input_translate_event(player[0].jselect));
	g_key_file_set_string(inputfile, "gamepad1", "js_start", input_translate_event(player[0].jstart));
	g_key_file_set_string(inputfile, "gamepad1", "js_a", input_translate_event(player[0].ja));
	g_key_file_set_string(inputfile, "gamepad1", "js_b", input_translate_event(player[0].jb));

	g_key_file_set_string(inputfile, "gamepad2", "kb_u", SDL_GetScancodeName(player[1].u));
	g_key_file_set_string(inputfile, "gamepad2", "kb_d", SDL_GetScancodeName(player[1].d));
	g_key_file_set_string(inputfile, "gamepad2", "kb_l", SDL_GetScancodeName(player[1].l));
	g_key_file_set_string(inputfile, "gamepad2", "kb_r", SDL_GetScancodeName(player[1].r));
	g_key_file_set_string(inputfile, "gamepad2", "kb_select", SDL_GetScancodeName(player[1].select));
	g_key_file_set_string(inputfile, "gamepad2", "kb_start", SDL_GetScancodeName(player[1].start));
	g_key_file_set_string(inputfile, "gamepad2", "kb_a", SDL_GetScancodeName(player[1].a));
	g_key_file_set_string(inputfile, "gamepad2", "kb_b", SDL_GetScancodeName(player[1].b));

	g_key_file_set_string(inputfile, "gamepad2", "js_u", input_translate_event(player[1].ju));
	g_key_file_set_string(inputfile, "gamepad2", "js_d", input_translate_event(player[1].jd));
	g_key_file_set_string(inputfile, "gamepad2", "js_l", input_translate_event(player[1].jl));
	g_key_file_set_string(inputfile, "gamepad2", "js_r", input_translate_event(player[1].jr));
	g_key_file_set_string(inputfile, "gamepad2", "js_select", input_translate_event(player[1].jselect));
	g_key_file_set_string(inputfile, "gamepad2", "js_start", input_translate_event(player[1].jstart));
	g_key_file_set_string(inputfile, "gamepad2", "js_a", input_translate_event(player[1].ja));
	g_key_file_set_string(inputfile, "gamepad2", "js_b", input_translate_event(player[1].jb));
	
	FILE *fp = fopen(inputconfpath, "w");
	if (fp != NULL)	{
		fputs(g_key_file_to_data(inputfile, &length, NULL), fp);
		fclose(fp);
	}
	
	g_slice_free(inputsettings, inputconf);
	g_key_file_free(inputfile);
}

void input_set_default() {
	// Set default input config	
	player[0].u = SDL_GetScancodeFromName("Up");
	player[0].d = SDL_GetScancodeFromName("Down");
	player[0].l = SDL_GetScancodeFromName("Left");
	player[0].r = SDL_GetScancodeFromName("Right");
	player[0].select = SDL_GetScancodeFromName("Right Shift");
	player[0].start = SDL_GetScancodeFromName("Right Ctrl");
	player[0].a = SDL_GetScancodeFromName("Z");
	player[0].b = SDL_GetScancodeFromName("A");

	player[0].ju = input_translate_string("j0h01");
	player[0].jd = input_translate_string("j0h04");
	player[0].jl = input_translate_string("j0h08");
	player[0].jr = input_translate_string("j0h02");
	player[0].jselect = input_translate_string("j0b8");
	player[0].jstart = input_translate_string("j0b9");
	player[0].ja = input_translate_string("j0b1");
	player[0].jb = input_translate_string("j0b0");
	
	player[1].u = SDL_GetScancodeFromName("I");
	player[1].d = SDL_GetScancodeFromName("K");
	player[1].l = SDL_GetScancodeFromName("J");
	player[1].r = SDL_GetScancodeFromName("L");
	player[1].select = SDL_GetScancodeFromName("Left Shift");
	player[1].start = SDL_GetScancodeFromName("Left Ctrl");
	player[1].a = SDL_GetScancodeFromName("M");
	player[1].b = SDL_GetScancodeFromName("N");
	
	player[1].ju = input_translate_string("j1h01");
	player[1].jd = input_translate_string("j1h04");
	player[1].jl = input_translate_string("j1h08");
	player[1].jr = input_translate_string("j1h02");
	player[1].jselect = input_translate_string("j1b8");
	player[1].jstart = input_translate_string("j1b9");
	player[1].ja = input_translate_string("j1b1");
	player[1].jb = input_translate_string("j1b0");
}