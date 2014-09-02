/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2014 R. Danbrook
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

#ifdef _GTK
#include "gtkui/gtkui.h"
#include "gtkui/gtkui_config.h"
#endif

#include "main.h"
#include "config.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "ini.h"

extern settings_t conf;
extern nstpaths_t nstpaths;
extern dimensions_t rendersize;
extern SDL_DisplayMode displaymode;
extern Emulator emulator;

bool confrunning, kbactivate = false;

inputsettings_t inputconf;
gamepad_t player[NUMGAMEPADS];

char inputconfpath[256];

turbo_t turbostate;
turbo_t turbotoggle;

SDL_Joystick *joystick;

static unsigned char nescodes[TOTALBUTTONS] = {
	Input::Controllers::Pad::UP,
	Input::Controllers::Pad::DOWN,
	Input::Controllers::Pad::LEFT,
	Input::Controllers::Pad::RIGHT,
	Input::Controllers::Pad::SELECT,
	Input::Controllers::Pad::START,
	Input::Controllers::Pad::A,
	Input::Controllers::Pad::B,
	Input::Controllers::Pad::A,
	Input::Controllers::Pad::B,
	Input::Controllers::Pad::UP,
	Input::Controllers::Pad::DOWN,
	Input::Controllers::Pad::LEFT,
	Input::Controllers::Pad::RIGHT,
	Input::Controllers::Pad::SELECT,
	Input::Controllers::Pad::START,
	Input::Controllers::Pad::A,
	Input::Controllers::Pad::B,
	Input::Controllers::Pad::A,
	Input::Controllers::Pad::B
};

void input_init() {
	// Initialize input
	Input(emulator).AutoSelectController(0);
	Input(emulator).AutoSelectController(1);
	video_set_cursor();
}

void input_joysticks_detect() {
	// Initialize any joysticks
	printf("%i joystick(s) found:\n", SDL_NumJoysticks());
	
	int i;
	
	for (i = 0; i < SDL_NumJoysticks(); i++) {
		joystick = SDL_JoystickOpen(i);
		printf("%s\n", SDL_JoystickName(joystick));
	}

	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	
	turbostate.p1a = turbotoggle.p1a = 0;
	turbostate.p1b = turbotoggle.p1b = 0;
	turbostate.p2a = turbotoggle.p2a = 0;
	turbostate.p2b = turbotoggle.p2b = 0;
}

void input_joysticks_close() {
	// Deinitialize any joysticks
	SDL_JoystickClose(joystick);
}

void input_process(Input::Controllers *controllers, SDL_Event event) {
	// Process input events
	
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

		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			input_match_mouse(controllers, event);
			break;
			
		default: break;
	}
}

void input_pulse_turbo(Input::Controllers *controllers) {
	// Pulse the turbo buttons if they're pressed
	if (turbostate.p1a) {
		turbotoggle.p1a++;
		if (turbotoggle.p1a >= conf.timing_turbopulse) {
			turbotoggle.p1a = 0;
			controllers->pad[0].buttons &= ~Input::Controllers::Pad::A;
		}
		else { controllers->pad[0].buttons |= Input::Controllers::Pad::A; }
	}
	
	if (turbostate.p1b) {
		turbotoggle.p1b++;
		if (turbotoggle.p1b >= conf.timing_turbopulse) {
			turbotoggle.p1b = 0;
			controllers->pad[0].buttons &= ~Input::Controllers::Pad::B;
		}
		else { controllers->pad[0].buttons |= Input::Controllers::Pad::B; }
	}
	
	if (turbostate.p2a) {
		turbotoggle.p2a++;
		if (turbotoggle.p2a >= conf.timing_turbopulse) {
			turbotoggle.p2a = 0;
			controllers->pad[1].buttons &= ~Input::Controllers::Pad::A;
		}
		else { controllers->pad[1].buttons |= Input::Controllers::Pad::A; }
	}
	
	if (turbostate.p2b) {
		turbotoggle.p2b++;
		if (turbotoggle.p2b >= conf.timing_turbopulse) {
			turbotoggle.p2b = 0;
			controllers->pad[1].buttons &= ~Input::Controllers::Pad::B;
		}
		else { controllers->pad[1].buttons |= Input::Controllers::Pad::B; }
	}
}

void input_inject(Input::Controllers *controllers, nesinput_t input) {
	// Insert the input signal into the NES
	if (input.pressed) {
		controllers->pad[input.player].buttons |= input.nescode;
		
		if (input.turboa) { input.player == 0 ? turbostate.p1a = true : turbostate.p2a = true; }
		if (input.turbob) { input.player == 0 ? turbostate.p1b = true : turbostate.p2b = true; }
	}
	else {
		controllers->pad[input.player].buttons &= ~input.nescode;
		
		if (input.turboa) { input.player == 0 ? turbostate.p1a = false : turbostate.p2a = false; }
		if (input.turbob) { input.player == 0 ? turbostate.p1b = false : turbostate.p2b = false; }
	}
}

void input_match_joystick(Input::Controllers *controllers, SDL_Event event) {
	// Match NES buttons to joystick input
	int j;
	
	nesinput_t input, reverseinput;
	
	input.nescode = 0x00;
	input.player = 0;
	input.pressed = 0;
	input.turboa = 0;
	input.turbob = 0;
	
	// This is for releasing opposing directions
	reverseinput.nescode = 0x00;
	reverseinput.player = 0;
	reverseinput.pressed = 0;
	
	SDL_Event buttons[TOTALBUTTONS] = {
		player[0].ju, player[0].jd, player[0].jl, player[0].jr,
		player[0].jselect, player[0].jstart, player[0].ja, player[0].jb,
		player[0].jta, player[0].jtb,
		
		player[1].ju, player[1].jd, player[1].jl, player[1].jr,
		player[1].jselect, player[1].jstart, player[1].ja, player[1].jb,
		player[1].jta, player[1].jtb
	};
	
	switch(event.type) {
		// Handle button input
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
			for (j = 0; j < TOTALBUTTONS; j++) {
				if (buttons[j].jbutton.button == event.jbutton.button
					&& buttons[j].jbutton.which == event.jbutton.which) {
					input.nescode = nescodes[j];
					if (j >= NUMBUTTONS) { input.player = 1; }
					// This is really dirty
					if (j == 8 || j == 18) { input.turboa = 1; }
					if (j == 9 || j == 19) { input.turbob = 1; }
				}
			}
			input.pressed = event.jbutton.state;
			break;
		
		// Handling hat input can be a lot of fun if you like pain
		case SDL_JOYHATMOTION:
			unsigned char hu, hd, hl, hr;
			hu = hd = hl = hr = 0;
			
			// Start a loop to check if input matches
			for (j = 0; j < TOTALBUTTONS; j++) {
				
				// Read value of each hat direction on current hat
				if (buttons[j].type == event.type
					&& buttons[j].jhat.which == event.jhat.which
					&& buttons[j].jhat.hat == event.jhat.hat) {
					if (j >= NUMBUTTONS) { input.player = reverseinput.player = 1; }

					// Find the values at each hat position on the current hat
					if (buttons[j].jhat.value == SDL_HAT_UP) { hu = nescodes[j]; }
					else if (buttons[j].jhat.value == SDL_HAT_DOWN) { hd = nescodes[j]; }
					else if (buttons[j].jhat.value == SDL_HAT_LEFT) { hl = nescodes[j]; }
					else if (buttons[j].jhat.value == SDL_HAT_RIGHT) { hr = nescodes[j]; }
					
					input.pressed = 1;
					
					// Make sure opposing hat positions are turned off
					switch(event.jhat.value) {
						case SDL_HAT_UP:
							input.nescode |= hu;
							reverseinput.nescode |= hd |= hl |= hr;
							break;
						case SDL_HAT_LEFTUP:
							input.nescode |= hu |= hl;
							reverseinput.nescode |= hd |= hr;
							break;
						case SDL_HAT_RIGHTUP:
							input.nescode |= hu |= hr;
							reverseinput.nescode |= hd |= hl;
							break;
						case SDL_HAT_DOWN:
							input.nescode |= hd;
							reverseinput.nescode |= hu |= hl |= hr;
							break;
						case SDL_HAT_LEFTDOWN:
							input.nescode |= hd |= hl;
							reverseinput.nescode |= hu |= hr;
							break;
						case SDL_HAT_RIGHTDOWN:
							input.nescode |= hd |= hr;
							reverseinput.nescode |= hu |= hl;
							break;
						case SDL_HAT_LEFT:
							input.nescode |= hl;
							reverseinput.nescode |= hr |= hu |= hd;
							break;
						case SDL_HAT_RIGHT:
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
			for (j = 0; j < TOTALBUTTONS; j++) {
				
				int nvalue = input_checksign(event.jaxis.value);

				if (buttons[j].jaxis.axis == event.jaxis.axis
					&& buttons[j].jaxis.which == event.jaxis.which
					&& buttons[j].jaxis.type == event.jaxis.type
					&& buttons[j].jaxis.value == nvalue) {

					if (j >= NUMBUTTONS) { input.player = reverseinput.player = 1; }
					
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
	
	nesinput_t input;

	input.nescode = 0x00;
	input.player = 0;
	input.pressed = 0;
	input.turboa = 0;
	input.turbob = 0;

	if (event.type == SDL_KEYDOWN) { input.pressed = 1; }

	for (int i = 0; i < NUMGAMEPADS; i++) {
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
		else if (player[i].ta == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::A;
			input.player = i;
			input.turboa = 1;
		}
		else if (player[i].tb == event.key.keysym.scancode) {
			input.nescode = Input::Controllers::Pad::B;
			input.player = i;
			input.turbob = 1;
		}
	}

	input_inject(controllers, input);
	
	if (event.key.keysym.scancode == SDL_SCANCODE_GRAVE && event.type == SDL_KEYDOWN) { timing_set_altspeed(); }
	if (event.key.keysym.scancode == SDL_SCANCODE_GRAVE && event.type == SDL_KEYUP) { timing_set_default(); }
	
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	
	// Insert Coins
	controllers->vsSystem.insertCoin = 0;
	if (keys[SDL_SCANCODE_F1]) { controllers->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_1; }
	if (keys[SDL_SCANCODE_F2]) { controllers->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_2; }
	
	// Process non-game events
	if (keys[SDL_SCANCODE_F3]) { nst_flip_disk(); }
	if (keys[SDL_SCANCODE_F4]) { nst_switch_disk(); }
	if (keys[SDL_SCANCODE_F5]) { nst_state_quicksave(0); }
	if (keys[SDL_SCANCODE_F6]) { nst_state_quicksave(1); }
	if (keys[SDL_SCANCODE_F7]) { nst_state_quickload(0); }
	if (keys[SDL_SCANCODE_F8]) { nst_state_quickload(1); }
	//if (keys[SDL_SCANCODE_F9]) {  }
	//if (keys[SDL_SCANCODE_F10]) {  }
	//if (keys[SDL_SCANCODE_F11]) {  }
	if (keys[SDL_SCANCODE_F12]) { nst_reset(0); }
	
	// Rewinder
	if (keys[SDL_SCANCODE_BACKSPACE]) { nst_set_rewind(0); }
	if (keys[SDL_SCANCODE_BACKSLASH]) { nst_set_rewind(1); }
	
	// Escape exits when not in GUI mode
	if (keys[SDL_SCANCODE_ESCAPE]) {
		if (conf.misc_disable_gui) { nst_schedule_quit(); }
	}
	
	// F toggles fullscreen
	if (keys[SDL_SCANCODE_F]) { video_toggle_fullscreen(); }
	if (keys[SDL_SCANCODE_T]) { video_toggle_filter(); }
	if (keys[SDL_SCANCODE_G]) { video_toggle_scalefactor(); }
}

void input_match_mouse(Input::Controllers *controllers, SDL_Event event) {
	// Match mouse input to NES input
	int x, y;
	double xaspect;
	double yaspect;
	#ifdef _GTK
	if (conf.misc_disable_gui) { SDL_GetMouseState(&x, &y); }
	else {
		x = event.button.x;
		y = event.button.y;
	}
	#else
	SDL_GetMouseState(&x, &y);
	#endif
	
	switch(event.type) {
		case SDL_MOUSEBUTTONUP:
			controllers->zapper.fire = false;
			break;
			
		case SDL_MOUSEBUTTONDOWN:
			// Get X coords
			if (conf.video_filter == 1) { // NTSC
				xaspect = (double)(Video::Output::WIDTH) / (double)(Video::Output::NTSC_WIDTH / 2);
			}
			else if (conf.video_tv_aspect) {
				xaspect = (double)(Video::Output::WIDTH) / (double)(TV_WIDTH);
			}
			else { xaspect = 1.0; }
			
			// Calculate fullscreen X coords
			if (conf.video_fullscreen) {
				if (conf.video_stretch_aspect) {
					xaspect = (double)(conf.video_scale_factor * Video::Output::WIDTH) / (double)(displaymode.w);
				}
				else {
					// Remove the same amount of pixels as the black area to the left of the screen
					x -= displaymode.w / 2.0f - rendersize.w / 2.0f;
					xaspect = (double)(conf.video_scale_factor * Video::Output::WIDTH) / (double)(rendersize.w);
				}
			}
			controllers->zapper.x = (int)(x * xaspect) / conf.video_scale_factor;
			
			// Get Y coords
			if (conf.video_unmask_overscan) {
				controllers->zapper.y = y / conf.video_scale_factor;
			}
			else {
				controllers->zapper.y = (y + OVERSCAN_TOP * conf.video_scale_factor) / conf.video_scale_factor;
			}
			
			// Calculate fullscreen Y coords
			if (conf.video_fullscreen) {
				yaspect = (double)(conf.video_scale_factor * Video::Output::HEIGHT) / (double)(displaymode.h);
				controllers->zapper.y = (y * yaspect) / conf.video_scale_factor;
			}
			
			// Offscreen
			if(event.button.button != SDL_BUTTON_LEFT) { controllers->zapper.x = ~1U; }
			
			controllers->zapper.fire = true;
			break;
			
		default: break;
	}
}

char* input_translate_event(SDL_Event event) {
	// Translate an SDL_Event to an inputcode
	static char inputcode[6];
	
	switch(event.type) {
		case SDL_JOYAXISMOTION:
			sprintf(inputcode, "j%da%d%d", event.jaxis.which, event.jaxis.axis, input_checksign(event.jaxis.value));
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
	
	int type, which, axis, value;
	
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
	
	if (axisvalue <= 0) { return 0; }
	else { return 1; }
}

void input_config_read() {
	// Read the input config file
	snprintf(inputconfpath, sizeof(inputconfpath), "%sinput.conf", nstpaths.nstdir);
	
	if (ini_parse(inputconfpath, input_config_match, &inputconf) < 0) {
		fprintf(stderr, "Failed to load input config file %s: Using defaults.\n", inputconfpath);
	}
	else {
		// Map the input settings from the config file
		
		// Player 1
		player[0].u = SDL_GetScancodeFromName(inputconf.kb_p1u);
		player[0].d = SDL_GetScancodeFromName(inputconf.kb_p1d);
		player[0].l = SDL_GetScancodeFromName(inputconf.kb_p1l);
		player[0].r = SDL_GetScancodeFromName(inputconf.kb_p1r);
		player[0].select = SDL_GetScancodeFromName(inputconf.kb_p1select);
		player[0].start = SDL_GetScancodeFromName(inputconf.kb_p1start);
		player[0].a = SDL_GetScancodeFromName(inputconf.kb_p1a);
		player[0].b = SDL_GetScancodeFromName(inputconf.kb_p1b);
		player[0].ta = SDL_GetScancodeFromName(inputconf.kb_p1ta);
		player[0].tb = SDL_GetScancodeFromName(inputconf.kb_p1tb);
		
		player[0].ju = input_translate_string(inputconf.js_p1u);
		player[0].jd = input_translate_string(inputconf.js_p1d);
		player[0].jl = input_translate_string(inputconf.js_p1l);
		player[0].jr = input_translate_string(inputconf.js_p1r);
		player[0].jselect = input_translate_string(inputconf.js_p1select);
		player[0].jstart = input_translate_string(inputconf.js_p1start);
		player[0].ja = input_translate_string(inputconf.js_p1a);
		player[0].jb = input_translate_string(inputconf.js_p1b);
		player[0].jta = input_translate_string(inputconf.js_p1ta);
		player[0].jtb = input_translate_string(inputconf.js_p1tb);
		
		// Player 2
		player[1].u = SDL_GetScancodeFromName(inputconf.kb_p2u);
		player[1].d = SDL_GetScancodeFromName(inputconf.kb_p2d);
		player[1].l = SDL_GetScancodeFromName(inputconf.kb_p2l);
		player[1].r = SDL_GetScancodeFromName(inputconf.kb_p2r);
		player[1].select = SDL_GetScancodeFromName(inputconf.kb_p2select);
		player[1].start = SDL_GetScancodeFromName(inputconf.kb_p2start);
		player[1].a = SDL_GetScancodeFromName(inputconf.kb_p2a);
		player[1].b = SDL_GetScancodeFromName(inputconf.kb_p2b);
		player[1].ta = SDL_GetScancodeFromName(inputconf.kb_p2ta);
		player[1].tb = SDL_GetScancodeFromName(inputconf.kb_p2tb);
		
		player[1].ju = input_translate_string(inputconf.js_p2u);
		player[1].jd = input_translate_string(inputconf.js_p2d);
		player[1].jl = input_translate_string(inputconf.js_p2l);
		player[1].jr = input_translate_string(inputconf.js_p2r);
		player[1].jselect = input_translate_string(inputconf.js_p2select);
		player[1].jstart = input_translate_string(inputconf.js_p2start);
		player[1].ja = input_translate_string(inputconf.js_p2a);
		player[1].jb = input_translate_string(inputconf.js_p2b);
		player[1].jta = input_translate_string(inputconf.js_p2ta);
		player[1].jtb = input_translate_string(inputconf.js_p2tb);
	}
}

void input_config_write() {
	// Write out the input configuration file
	
	FILE *fp = fopen(inputconfpath, "w");
	if (fp != NULL)	{
		fprintf(fp, "; Nestopia UE Input Configuration File\n\n");
		fprintf(fp, "; Possible values for keyboard input are in the Key Name column:\n; https://wiki.libsdl.org/SDL_Scancode\n\n");
		fprintf(fp, "; Possible values for joystick input:\n; j[joystick number][a|b|h][button/hat/axis number][1/0 = +/- (axes only)]\n");
		fprintf(fp, "; Example: j0b3 = joystick 0, button 3. j1a11 = joystick 1, axis 1 +\n\n");
		fprintf(fp, "[gamepad1]\n");
		fprintf(fp, "kb_u=%s\n", SDL_GetScancodeName(player[0].u));
		fprintf(fp, "kb_d=%s\n", SDL_GetScancodeName(player[0].d));
		fprintf(fp, "kb_l=%s\n", SDL_GetScancodeName(player[0].l));
		fprintf(fp, "kb_r=%s\n", SDL_GetScancodeName(player[0].r));
		fprintf(fp, "kb_select=%s\n", SDL_GetScancodeName(player[0].select));
		fprintf(fp, "kb_start=%s\n", SDL_GetScancodeName(player[0].start));
		fprintf(fp, "kb_a=%s\n", SDL_GetScancodeName(player[0].a));
		fprintf(fp, "kb_b=%s\n", SDL_GetScancodeName(player[0].b));
		fprintf(fp, "kb_ta=%s\n", SDL_GetScancodeName(player[0].ta));
		fprintf(fp, "kb_tb=%s\n", SDL_GetScancodeName(player[0].tb));
		
		fprintf(fp, "js_u=%s\n", input_translate_event(player[0].ju));
		fprintf(fp, "js_d=%s\n", input_translate_event(player[0].jd));
		fprintf(fp, "js_l=%s\n", input_translate_event(player[0].jl));
		fprintf(fp, "js_r=%s\n", input_translate_event(player[0].jr));
		fprintf(fp, "js_select=%s\n", input_translate_event(player[0].jselect));
		fprintf(fp, "js_start=%s\n", input_translate_event(player[0].jstart));
		fprintf(fp, "js_a=%s\n", input_translate_event(player[0].ja));
		fprintf(fp, "js_b=%s\n", input_translate_event(player[0].jb));
		fprintf(fp, "js_ta=%s\n", input_translate_event(player[0].jta));
		fprintf(fp, "js_tb=%s\n", input_translate_event(player[0].jtb));
		fprintf(fp, "\n"); // End of Section
		
		fprintf(fp, "[gamepad2]\n");
		fprintf(fp, "kb_u=%s\n", SDL_GetScancodeName(player[1].u));
		fprintf(fp, "kb_d=%s\n", SDL_GetScancodeName(player[1].d));
		fprintf(fp, "kb_l=%s\n", SDL_GetScancodeName(player[1].l));
		fprintf(fp, "kb_r=%s\n", SDL_GetScancodeName(player[1].r));
		fprintf(fp, "kb_select=%s\n", SDL_GetScancodeName(player[1].select));
		fprintf(fp, "kb_start=%s\n", SDL_GetScancodeName(player[1].start));
		fprintf(fp, "kb_a=%s\n", SDL_GetScancodeName(player[1].a));
		fprintf(fp, "kb_b=%s\n", SDL_GetScancodeName(player[1].b));
		fprintf(fp, "kb_ta=%s\n", SDL_GetScancodeName(player[1].ta));
		fprintf(fp, "kb_tb=%s\n", SDL_GetScancodeName(player[1].tb));
		
		fprintf(fp, "js_u=%s\n", input_translate_event(player[1].ju));
		fprintf(fp, "js_d=%s\n", input_translate_event(player[1].jd));
		fprintf(fp, "js_l=%s\n", input_translate_event(player[1].jl));
		fprintf(fp, "js_r=%s\n", input_translate_event(player[1].jr));
		fprintf(fp, "js_select=%s\n", input_translate_event(player[1].jselect));
		fprintf(fp, "js_start=%s\n", input_translate_event(player[1].jstart));
		fprintf(fp, "js_a=%s\n", input_translate_event(player[1].ja));
		fprintf(fp, "js_b=%s\n", input_translate_event(player[1].jb));
		fprintf(fp, "js_ta=%s\n", input_translate_event(player[1].jta));
		fprintf(fp, "js_tb=%s\n", input_translate_event(player[1].jtb));
		fprintf(fp, "\n"); // End of Section
		
		fclose(fp);
	}
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
	player[0].ta = SDL_GetScancodeFromName("X");
	player[0].tb = SDL_GetScancodeFromName("S");

	player[0].ju = input_translate_string("j0h01");
	player[0].jd = input_translate_string("j0h04");
	player[0].jl = input_translate_string("j0h08");
	player[0].jr = input_translate_string("j0h02");
	player[0].jselect = input_translate_string("j0b8");
	player[0].jstart = input_translate_string("j0b9");
	player[0].ja = input_translate_string("j0b1");
	player[0].jb = input_translate_string("j0b0");
	player[0].jta = input_translate_string("j0b2");
	player[0].jtb = input_translate_string("j0b3");
	
	player[1].u = SDL_GetScancodeFromName("I");
	player[1].d = SDL_GetScancodeFromName("K");
	player[1].l = SDL_GetScancodeFromName("J");
	player[1].r = SDL_GetScancodeFromName("L");
	player[1].select = SDL_GetScancodeFromName("Left Shift");
	player[1].start = SDL_GetScancodeFromName("Left Ctrl");
	player[1].a = SDL_GetScancodeFromName("M");
	player[1].b = SDL_GetScancodeFromName("N");
	player[1].ta = SDL_GetScancodeFromName("B");
	player[1].tb = SDL_GetScancodeFromName("V");
	
	player[1].ju = input_translate_string("j1h01");
	player[1].jd = input_translate_string("j1h04");
	player[1].jl = input_translate_string("j1h08");
	player[1].jr = input_translate_string("j1h02");
	player[1].jselect = input_translate_string("j1b8");
	player[1].jstart = input_translate_string("j1b9");
	player[1].ja = input_translate_string("j1b1");
	player[1].jb = input_translate_string("j1b0");
	player[1].jta = input_translate_string("j1b2");
	player[1].jtb = input_translate_string("j1b3");
}

static int input_config_match(void* user, const char* section, const char* name, const char* value) {
	// Match values from input config file and populate live config
	inputsettings_t* pconfig = (inputsettings_t*)user;
	
	// Player 1
	if (MATCH("gamepad1", "kb_u")) { pconfig->kb_p1u = strdup(value); }
	else if (MATCH("gamepad1", "kb_d")) { pconfig->kb_p1d = strdup(value); }
	else if (MATCH("gamepad1", "kb_l")) { pconfig->kb_p1l = strdup(value); }
	else if (MATCH("gamepad1", "kb_r")) { pconfig->kb_p1r = strdup(value); }
	else if (MATCH("gamepad1", "kb_select")) { pconfig->kb_p1select = strdup(value); }
	else if (MATCH("gamepad1", "kb_start")) { pconfig->kb_p1start = strdup(value); }
	else if (MATCH("gamepad1", "kb_a")) { pconfig->kb_p1a = strdup(value); }
	else if (MATCH("gamepad1", "kb_b")) { pconfig->kb_p1b = strdup(value); }
	else if (MATCH("gamepad1", "kb_ta")) { pconfig->kb_p1ta = strdup(value); }
	else if (MATCH("gamepad1", "kb_tb")) { pconfig->kb_p1tb = strdup(value); }
	
	else if (MATCH("gamepad1", "js_u")) { pconfig->js_p1u = strdup(value); }
	else if (MATCH("gamepad1", "js_d")) { pconfig->js_p1d = strdup(value); }
	else if (MATCH("gamepad1", "js_l")) { pconfig->js_p1l = strdup(value); }
	else if (MATCH("gamepad1", "js_r")) { pconfig->js_p1r = strdup(value); }
	else if (MATCH("gamepad1", "js_select")) { pconfig->js_p1select = strdup(value); }
	else if (MATCH("gamepad1", "js_start")) { pconfig->js_p1start = strdup(value); }
	else if (MATCH("gamepad1", "js_a")) { pconfig->js_p1a = strdup(value); }
	else if (MATCH("gamepad1", "js_b")) { pconfig->js_p1b = strdup(value); }
	else if (MATCH("gamepad1", "js_ta")) { pconfig->js_p1ta = strdup(value); }
	else if (MATCH("gamepad1", "js_tb")) { pconfig->js_p1tb = strdup(value); }
	
	// Player 2
	else if (MATCH("gamepad2", "kb_u")) { pconfig->kb_p2u = strdup(value); }
	else if (MATCH("gamepad2", "kb_d")) { pconfig->kb_p2d = strdup(value); }
	else if (MATCH("gamepad2", "kb_l")) { pconfig->kb_p2l = strdup(value); }
	else if (MATCH("gamepad2", "kb_r")) { pconfig->kb_p2r = strdup(value); }
	else if (MATCH("gamepad2", "kb_select")) { pconfig->kb_p2select = strdup(value); }
	else if (MATCH("gamepad2", "kb_start")) { pconfig->kb_p2start = strdup(value); }
	else if (MATCH("gamepad2", "kb_a")) { pconfig->kb_p2a = strdup(value); }
	else if (MATCH("gamepad2", "kb_b")) { pconfig->kb_p2b = strdup(value); }
	else if (MATCH("gamepad2", "kb_ta")) { pconfig->kb_p2ta = strdup(value); }
	else if (MATCH("gamepad2", "kb_tb")) { pconfig->kb_p2tb = strdup(value); }
	
	else if (MATCH("gamepad2", "js_u")) { pconfig->js_p2u = strdup(value); }
	else if (MATCH("gamepad2", "js_d")) { pconfig->js_p2d = strdup(value); }
	else if (MATCH("gamepad2", "js_l")) { pconfig->js_p2l = strdup(value); }
	else if (MATCH("gamepad2", "js_r")) { pconfig->js_p2r = strdup(value); }
	else if (MATCH("gamepad2", "js_select")) { pconfig->js_p2select = strdup(value); }
	else if (MATCH("gamepad2", "js_start")) { pconfig->js_p2start = strdup(value); }
	else if (MATCH("gamepad2", "js_a")) { pconfig->js_p2a = strdup(value); }
	else if (MATCH("gamepad2", "js_b")) { pconfig->js_p2b = strdup(value); }
	else if (MATCH("gamepad2", "js_ta")) { pconfig->js_p2ta = strdup(value); }
	else if (MATCH("gamepad2", "js_tb")) { pconfig->js_p2tb = strdup(value); }
	
	else { return 0; }
    return 1;
}

int input_configure_item(int pnum, int bnum, int type) {
	// Configure an input item
	
	if (confrunning) { return 0; }
	
	SDL_Event event, eventbuf;
	
	int axis = 0, axisnoise = 0, counter = 0;
	
	// Enter and Space hack
	bool etoggle = true;
	if (kbactivate) {
		etoggle = false;
	}
	
	confrunning = true;
	bool confstop = false;
	while (confrunning) {
		#ifdef _GTK
		while (gtk_events_pending()) {
			gtk_main_iteration_do(FALSE);
		}
		#endif
		while(SDL_PollEvent(&event)) {
			// Time to quit?
			switch (event.key.keysym.sym) {
				case SDLK_ESCAPE:
					confrunning = false;
					break;
				default: break;
			}
			// Process the event
			if (type == 0) { // Keyboard
				switch(event.type) {
					case SDL_KEYUP:
						// Enter and Space need to be handled separately
						if (event.key.keysym.sym == SDLK_RETURN || event.key.keysym.sym == SDLK_SPACE) {
							if (etoggle) { input_set_item(event, type, pnum, bnum); }
							else { etoggle = true; break; }
						}
						else {
							input_set_item(event, type, pnum, bnum);
						}
						confstop = true;
						break;
					default: break;
				}
			}
			else if (type == 1) { // Joystick
				switch(event.type) {
					case SDL_JOYBUTTONDOWN:
						input_set_item(event, type, pnum, bnum);
						confstop = true;
						break;
					
					case SDL_JOYHATMOTION:
						if (event.jhat.value != SDL_HAT_CENTERED) {
							input_set_item(event, type, pnum, bnum);
							confstop = true;
						}
						break;
					
					case SDL_JOYAXISMOTION:
						if (abs(event.jaxis.value) >= DEADZONE) {
							eventbuf = event;
							axisnoise = 1;
							axis = event.jaxis.axis;
						}
						
						else if (abs(event.jaxis.value) < DEADZONE && axisnoise && event.jaxis.axis == axis) {
							input_set_item(event, type, pnum, bnum);
							axisnoise = 0;
							confstop = true;
						}						
						break;
					
					default: break;
				}
			}
		}
		if (confstop) { confrunning = false; }
	}
	
	return 1;
}

void input_set_item(SDL_Event event, int type, int pnum, int counter) {
	// Set an input item to what was requested by configuration process
	if (type == 0) { // Keyboard
		switch(counter) {
			case 0: player[pnum].u = event.key.keysym.scancode; break;
			case 1: player[pnum].d = event.key.keysym.scancode; break;
			case 2: player[pnum].l = event.key.keysym.scancode; break;
			case 3: player[pnum].r = event.key.keysym.scancode; break;
			case 4: player[pnum].select = event.key.keysym.scancode; break;
			case 5: player[pnum].start = event.key.keysym.scancode; break;
			case 6: player[pnum].a = event.key.keysym.scancode; break;
			case 7: player[pnum].b = event.key.keysym.scancode; break;
			case 8: player[pnum].ta = event.key.keysym.scancode; break;
			case 9: player[pnum].tb = event.key.keysym.scancode; break;
			default: break;
		}
	}
	else if (type == 1) { // Joystick
		switch(counter) {
			case 0: player[pnum].ju = input_translate_string(input_translate_event(event)); break;
			case 1: player[pnum].jd = input_translate_string(input_translate_event(event)); break;
			case 2: player[pnum].jl = input_translate_string(input_translate_event(event)); break;
			case 3: player[pnum].jr = input_translate_string(input_translate_event(event)); break;
			case 4: player[pnum].jselect = input_translate_string(input_translate_event(event)); break;
			case 5: player[pnum].jstart = input_translate_string(input_translate_event(event)); break;
			case 6: player[pnum].ja = input_translate_string(input_translate_event(event)); break;
			case 7: player[pnum].jb = input_translate_string(input_translate_event(event)); break;
			case 8: player[pnum].jta = input_translate_string(input_translate_event(event)); break;
			case 9: player[pnum].jtb = input_translate_string(input_translate_event(event)); break;
			default: break;
		}
	}
}
