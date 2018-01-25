/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2018 R. Danbrook
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

#include <SDL.h>
#ifdef _GTK
#include "gtkui/gtkui.h"
#endif

#include "nstcommon.h"
#include "video.h"
#include "input.h"

#include "ini.h"

#include "sdlmain.h"
#include "sdlvideo.h"
#include "sdlinput.h"

static SDL_Joystick *joystick;
gamepad_t player[NUMGAMEPADS];
static uiinput_t ui;
static inputsettings_t inputconf;
static char inputconfpath[256];
extern int drawtext;

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

extern Emulator emulator;
extern nstpaths_t nstpaths;

void nstsdl_input_joysticks_detect() {
	// Initialize any joysticks
	fprintf(stderr, "%i joystick(s) found:\n", SDL_NumJoysticks());
	
	int i;
	
	for (i = 0; i < SDL_NumJoysticks(); i++) {
		joystick = SDL_JoystickOpen(i);
		printf("%s\n", SDL_JoystickName(joystick));
	}

	SDL_SetHint(SDL_HINT_JOYSTICK_ALLOW_BACKGROUND_EVENTS, "1");
	
	nst_input_turbo_init();
}

void nstsdl_input_joysticks_close() {
	// Deinitialize any joysticks
	SDL_JoystickClose(joystick);
}

void nstsdl_input_match_keyboard(Input::Controllers *controllers, SDL_Event event) {
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

	nst_input_inject(controllers, input);
	
	if (event.key.keysym.scancode == ui.ffspeed && event.type == SDL_KEYDOWN) { nst_timing_set_ffspeed(); }
	if (event.key.keysym.scancode == ui.ffspeed && event.type == SDL_KEYUP) { nst_timing_set_default(); }
	
	const Uint8 *keys = SDL_GetKeyboardState(NULL);
	
	// Mic emulation
	if (keys[SDL_SCANCODE_SPACE]) { controllers->pad[1].mic = 0x04; }
	else { controllers->pad[1].mic = 0x00; }
	
	// Insert Coins
	controllers->vsSystem.insertCoin = 0;
	if (keys[ui.insertcoin1]) { controllers->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_1; }
	if (keys[ui.insertcoin2]) { controllers->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_2; }
	
	// Process non-game events
	if (keys[ui.fdsflip]) { nst_fds_flip(); }
	if (keys[ui.fdsswitch]) { nst_fds_switch(); }
	if (keys[ui.qsave1]) { nst_state_quicksave(0); }
	if (keys[ui.qsave2]) { nst_state_quicksave(1); }
	if (keys[ui.qload1]) { nst_state_quickload(0); }
	if (keys[ui.qload2]) { nst_state_quickload(1); }
	
	// Screenshot
	if (keys[ui.screenshot]) { video_screenshot(NULL); }
	
	// Reset
	if (keys[ui.reset]) { nst_reset(0); }
	
	// Rewinder
	if (keys[ui.rwstart]) { nst_set_rewind(0); }
	if (keys[ui.rwstop]) { nst_set_rewind(1); }
	
	// Video
	if (event.key.keysym.scancode == ui.fullscreen && event.type == SDL_KEYUP) { nstsdl_video_toggle_fullscreen(); }
	if (keys[ui.filter]) { nstsdl_video_toggle_filter(); }
	if (keys[ui.scalefactor]) { nstsdl_video_toggle_scale(); }
	
	// NSF
	if (nst_nsf()) {
		if (keys[SDL_SCANCODE_UP]) { nst_nsf_play(); }
		if (keys[SDL_SCANCODE_DOWN]) { nst_nsf_stop(); }
		if (keys[SDL_SCANCODE_LEFT]) { nst_nsf_prev(); }
		if (keys[SDL_SCANCODE_RIGHT]) { nst_nsf_next(); }
	}
	
	// Input Config
	if (keys[SDL_SCANCODE_LCTRL] && keys[SDL_SCANCODE_1]) { nstsdl_input_conf(0, 0); } // Keyboard Player 1
	if (keys[SDL_SCANCODE_LCTRL] && keys[SDL_SCANCODE_2]) { nstsdl_input_conf(0, 1); } // Keyboard Player 2
	if (keys[SDL_SCANCODE_LSHIFT] && keys[SDL_SCANCODE_1]) { nstsdl_input_conf(1, 0); } // Joystick Player 1
	if (keys[SDL_SCANCODE_LSHIFT] && keys[SDL_SCANCODE_2]) { nstsdl_input_conf(1, 1); } // Joystick Player 2
	
	// Escape exits
#ifndef _GTK	
	if (keys[SDL_SCANCODE_ESCAPE]) { nst_schedule_quit(); }
#endif
}

void nstsdl_input_match_mouse(Input::Controllers *controllers, SDL_Event event) {
	// Match mouse input to NES input
	int x, y;
	SDL_GetMouseState(&x, &y);
	nst_input_inject_mouse(controllers, event.button.button, event.type == SDL_MOUSEBUTTONDOWN ? 1 : 0, x, y);
}

int nstsdl_input_checksign(int axisvalue) {
	if (axisvalue <= 0) { return 0; }
	else { return 1; }
}

void nstsdl_input_match_joystick(Input::Controllers *controllers, SDL_Event event) {
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
	
	SDL_Event rw[2] = { player[0].rwstart, player[0].rwstop };
	
	switch(event.type) {
		// Handle button input
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
			// Gamepad input
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
			
			// Rewind
			if (event.jbutton.button == rw[0].jbutton.button && event.jbutton.which == rw[0].jbutton.which) { nst_set_rewind(0); }
			if (event.jbutton.button == rw[1].jbutton.button && event.jbutton.which == rw[1].jbutton.which) { nst_set_rewind(1); }
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
				
				int nvalue = nstsdl_input_checksign(event.jaxis.value);

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
	
	nst_input_inject(controllers, reverseinput);
	nst_input_inject(controllers, input);
}

void nstsdl_input_conf_defaults() {
	// Set default input config
	ui.qsave1 = SDL_GetScancodeFromName("F5");
	ui.qsave2 = SDL_GetScancodeFromName("F6");
	ui.qload1 = SDL_GetScancodeFromName("F7");
	ui.qload2 = SDL_GetScancodeFromName("F8");
	
	ui.screenshot = SDL_GetScancodeFromName("F9");
	
	ui.fdsflip = SDL_GetScancodeFromName("F3");
	ui.fdsswitch = SDL_GetScancodeFromName("F4");
	
	ui.insertcoin1 = SDL_GetScancodeFromName("F1");
	ui.insertcoin2 = SDL_GetScancodeFromName("F2");
	
	ui.reset = SDL_GetScancodeFromName("F12");
	
	ui.ffspeed = SDL_GetScancodeFromName("`");
	ui.rwstart = SDL_GetScancodeFromName("Backspace");
	ui.rwstop = SDL_GetScancodeFromName("\\");
	
	ui.fullscreen = SDL_GetScancodeFromName("F");
	ui.filter = SDL_GetScancodeFromName("T");
	ui.scalefactor = SDL_GetScancodeFromName("G");
	
	player[0].u = SDL_GetScancodeFromName("Up");
	player[0].d = SDL_GetScancodeFromName("Down");
	player[0].l = SDL_GetScancodeFromName("Left");
	player[0].r = SDL_GetScancodeFromName("Right");
	player[0].select = SDL_GetScancodeFromName("Right Shift");
	player[0].start = SDL_GetScancodeFromName("Return");
	player[0].a = SDL_GetScancodeFromName("Z");
	player[0].b = SDL_GetScancodeFromName("A");
	player[0].ta = SDL_GetScancodeFromName("X");
	player[0].tb = SDL_GetScancodeFromName("S");

	player[0].ju = nstsdl_input_translate_string("j0h01");
	player[0].jd = nstsdl_input_translate_string("j0h04");
	player[0].jl = nstsdl_input_translate_string("j0h08");
	player[0].jr = nstsdl_input_translate_string("j0h02");
	player[0].jselect = nstsdl_input_translate_string("j0b8");
	player[0].jstart = nstsdl_input_translate_string("j0b9");
	player[0].ja = nstsdl_input_translate_string("j0b1");
	player[0].jb = nstsdl_input_translate_string("j0b0");
	player[0].jta = nstsdl_input_translate_string("j0b2");
	player[0].jtb = nstsdl_input_translate_string("j0b3");
	
	player[0].rwstart = nstsdl_input_translate_string("j0b4");
	player[0].rwstop = nstsdl_input_translate_string("j0b5");
	
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
	
	player[1].ju = nstsdl_input_translate_string("j1h01");
	player[1].jd = nstsdl_input_translate_string("j1h04");
	player[1].jl = nstsdl_input_translate_string("j1h08");
	player[1].jr = nstsdl_input_translate_string("j1h02");
	player[1].jselect = nstsdl_input_translate_string("j1b8");
	player[1].jstart = nstsdl_input_translate_string("j1b9");
	player[1].ja = nstsdl_input_translate_string("j1b1");
	player[1].jb = nstsdl_input_translate_string("j1b0");
	player[1].jta = nstsdl_input_translate_string("j1b2");
	player[1].jtb = nstsdl_input_translate_string("j1b3");
}

void nstsdl_input_conf_set(SDL_Event event, int type, int pnum, int counter) {
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
			case 0: player[pnum].ju = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 1: player[pnum].jd = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 2: player[pnum].jl = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 3: player[pnum].jr = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 4: player[pnum].jselect = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 5: player[pnum].jstart = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 6: player[pnum].ja = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 7: player[pnum].jb = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 8: player[pnum].jta = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			case 9: player[pnum].jtb = nstsdl_input_translate_string(nstsdl_input_translate_event(event)); break;
			default: break;
		}
	}
}

static int nstsdl_input_config_match(void* user, const char* section, const char* name, const char* value) {
	// Match values from input config file and populate live config
	inputsettings_t* pconfig = (inputsettings_t*)user;
	
	// User Interface
	if (MATCH("ui", "qsave1")) { pconfig->qsave1 = strdup(value); }
	else if (MATCH("ui", "qsave2")) { pconfig->qsave2 = strdup(value); }
	else if (MATCH("ui", "qload1")) { pconfig->qload1 = strdup(value); }
	else if (MATCH("ui", "qload2")) { pconfig->qload2 = strdup(value); }
	
	else if (MATCH("ui", "screenshot")) { pconfig->screenshot = strdup(value); }
	
	else if (MATCH("ui", "fdsflip")) { pconfig->fdsflip = strdup(value); }
	else if (MATCH("ui", "fdsswitch")) { pconfig->fdsswitch = strdup(value); }
	
	else if (MATCH("ui", "insertcoin1")) { pconfig->insertcoin1 = strdup(value); }
	else if (MATCH("ui", "insertcoin2")) { pconfig->insertcoin2 = strdup(value); }
	
	else if (MATCH("ui", "reset")) { pconfig->reset = strdup(value); }
	
	else if (MATCH("ui", "ffspeed")) { pconfig->ffspeed = strdup(value); }
	else if (MATCH("ui", "rwstart")) { pconfig->rwstart = strdup(value); }
	else if (MATCH("ui", "rwstop")) { pconfig->rwstop = strdup(value); }
	
	else if (MATCH("ui", "fullscreen")) { pconfig->fullscreen = strdup(value); }
	else if (MATCH("ui", "filter")) { pconfig->filter = strdup(value); }
	else if (MATCH("ui", "scalefactor")) { pconfig->scalefactor = strdup(value); }
	
	// Player 1
	else if (MATCH("gamepad1", "kb_u")) { pconfig->kb_p1u = strdup(value); }
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
	
	else if (MATCH("gamepad1", "js_rwstart")) { pconfig->js_rwstart = strdup(value); }
	else if (MATCH("gamepad1", "js_rwstop")) { pconfig->js_rwstop = strdup(value); }
	
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

void nstsdl_input_conf_read() {
	// Read the input config file
	snprintf(inputconfpath, sizeof(inputconfpath), "%sinput.conf", nstpaths.nstdir);
	
	if (ini_parse(inputconfpath, nstsdl_input_config_match, &inputconf) < 0) {
		fprintf(stderr, "Failed to load input config file %s: Using defaults.\n", inputconfpath);
	}
	else { // Map the input settings from the config file
		// User Interface
		ui.qsave1 = SDL_GetScancodeFromName(inputconf.qsave1);
		ui.qsave2 = SDL_GetScancodeFromName(inputconf.qsave2);
		ui.qload1 = SDL_GetScancodeFromName(inputconf.qload1);
		ui.qload2 = SDL_GetScancodeFromName(inputconf.qload2);
		
		ui.screenshot = SDL_GetScancodeFromName(inputconf.screenshot);
		
		ui.fdsflip = SDL_GetScancodeFromName(inputconf.fdsflip);
		ui.fdsswitch = SDL_GetScancodeFromName(inputconf.fdsswitch);
		
		ui.insertcoin1 = SDL_GetScancodeFromName(inputconf.insertcoin1);
		ui.insertcoin2 = SDL_GetScancodeFromName(inputconf.insertcoin2);
		
		ui.reset = SDL_GetScancodeFromName(inputconf.reset);
		
		ui.ffspeed = SDL_GetScancodeFromName(inputconf.ffspeed);
		ui.rwstart = SDL_GetScancodeFromName(inputconf.rwstart);
		ui.rwstop = SDL_GetScancodeFromName(inputconf.rwstop);
		
		ui.fullscreen = SDL_GetScancodeFromName(inputconf.fullscreen);
		ui.filter = SDL_GetScancodeFromName(inputconf.filter);
		ui.scalefactor = SDL_GetScancodeFromName(inputconf.scalefactor);
		
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
		
		player[0].ju = nstsdl_input_translate_string(inputconf.js_p1u);
		player[0].jd = nstsdl_input_translate_string(inputconf.js_p1d);
		player[0].jl = nstsdl_input_translate_string(inputconf.js_p1l);
		player[0].jr = nstsdl_input_translate_string(inputconf.js_p1r);
		player[0].jselect = nstsdl_input_translate_string(inputconf.js_p1select);
		player[0].jstart = nstsdl_input_translate_string(inputconf.js_p1start);
		player[0].ja = nstsdl_input_translate_string(inputconf.js_p1a);
		player[0].jb = nstsdl_input_translate_string(inputconf.js_p1b);
		player[0].jta = nstsdl_input_translate_string(inputconf.js_p1ta);
		player[0].jtb = nstsdl_input_translate_string(inputconf.js_p1tb);
		
		if (inputconf.js_rwstart) { player[0].rwstart = nstsdl_input_translate_string(inputconf.js_rwstart); }
		if (inputconf.js_rwstop) { player[0].rwstop = nstsdl_input_translate_string(inputconf.js_rwstop); }
		
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
		
		player[1].ju = nstsdl_input_translate_string(inputconf.js_p2u);
		player[1].jd = nstsdl_input_translate_string(inputconf.js_p2d);
		player[1].jl = nstsdl_input_translate_string(inputconf.js_p2l);
		player[1].jr = nstsdl_input_translate_string(inputconf.js_p2r);
		player[1].jselect = nstsdl_input_translate_string(inputconf.js_p2select);
		player[1].jstart = nstsdl_input_translate_string(inputconf.js_p2start);
		player[1].ja = nstsdl_input_translate_string(inputconf.js_p2a);
		player[1].jb = nstsdl_input_translate_string(inputconf.js_p2b);
		player[1].jta = nstsdl_input_translate_string(inputconf.js_p2ta);
		player[1].jtb = nstsdl_input_translate_string(inputconf.js_p2tb);
	}
}

void nstsdl_input_conf_write() {
	// Write out the input configuration file
	FILE *fp = fopen(inputconfpath, "w");
	if (fp != NULL)	{
		fprintf(fp, "; Nestopia UE SDL Input Configuration File\n\n");
		fprintf(fp, "; Possible values for keyboard input are in the Key Name column:\n; https://wiki.libsdl.org/SDL_Scancode\n\n");
		fprintf(fp, "; Possible values for joystick input:\n; j[joystick number][a|b|h][button/hat/axis number][1/0 = +/- (axes only)]\n");
		fprintf(fp, "; Example: j0b3 = joystick 0, button 3. j1a11 = joystick 1, axis 1 +\n\n");
		
		fprintf(fp, "[ui]\n");
		fprintf(fp, "qsave1=%s\n", SDL_GetScancodeName(ui.qsave1));
		fprintf(fp, "qsave2=%s\n", SDL_GetScancodeName(ui.qsave2));
		fprintf(fp, "qload1=%s\n", SDL_GetScancodeName(ui.qload1));
		fprintf(fp, "qload2=%s\n", SDL_GetScancodeName(ui.qload2));
		
		fprintf(fp, "screenshot=%s\n", SDL_GetScancodeName(ui.screenshot));
		
		fprintf(fp, "fdsflip=%s\n", SDL_GetScancodeName(ui.fdsflip));
		fprintf(fp, "fdsswitch=%s\n", SDL_GetScancodeName(ui.fdsswitch));
		
		fprintf(fp, "insertcoin1=%s\n", SDL_GetScancodeName(ui.insertcoin1));
		fprintf(fp, "insertcoin2=%s\n", SDL_GetScancodeName(ui.insertcoin2));
		
		fprintf(fp, "reset=%s\n", SDL_GetScancodeName(ui.reset));
		
		fprintf(fp, "ffspeed=%s\n", SDL_GetScancodeName(ui.ffspeed));
		fprintf(fp, "rwstart=%s\n", SDL_GetScancodeName(ui.rwstart));
		fprintf(fp, "rwstop=%s\n", SDL_GetScancodeName(ui.rwstop));
		
		fprintf(fp, "fullscreen=%s\n", SDL_GetScancodeName(ui.fullscreen));
		fprintf(fp, "filter=%s\n", SDL_GetScancodeName(ui.filter));
		fprintf(fp, "scalefactor=%s\n", SDL_GetScancodeName(ui.scalefactor));
		fprintf(fp, "\n"); // End of Section
		
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
		
		fprintf(fp, "js_u=%s\n", nstsdl_input_translate_event(player[0].ju));
		fprintf(fp, "js_d=%s\n", nstsdl_input_translate_event(player[0].jd));
		fprintf(fp, "js_l=%s\n", nstsdl_input_translate_event(player[0].jl));
		fprintf(fp, "js_r=%s\n", nstsdl_input_translate_event(player[0].jr));
		fprintf(fp, "js_select=%s\n", nstsdl_input_translate_event(player[0].jselect));
		fprintf(fp, "js_start=%s\n", nstsdl_input_translate_event(player[0].jstart));
		fprintf(fp, "js_a=%s\n", nstsdl_input_translate_event(player[0].ja));
		fprintf(fp, "js_b=%s\n", nstsdl_input_translate_event(player[0].jb));
		fprintf(fp, "js_ta=%s\n", nstsdl_input_translate_event(player[0].jta));
		fprintf(fp, "js_tb=%s\n", nstsdl_input_translate_event(player[0].jtb));
		
		fprintf(fp, "js_rwstart=%s\n", nstsdl_input_translate_event(player[0].rwstart));
		fprintf(fp, "js_rwstop=%s\n", nstsdl_input_translate_event(player[0].rwstop));
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
		
		fprintf(fp, "js_u=%s\n", nstsdl_input_translate_event(player[1].ju));
		fprintf(fp, "js_d=%s\n", nstsdl_input_translate_event(player[1].jd));
		fprintf(fp, "js_l=%s\n", nstsdl_input_translate_event(player[1].jl));
		fprintf(fp, "js_r=%s\n", nstsdl_input_translate_event(player[1].jr));
		fprintf(fp, "js_select=%s\n", nstsdl_input_translate_event(player[1].jselect));
		fprintf(fp, "js_start=%s\n", nstsdl_input_translate_event(player[1].jstart));
		fprintf(fp, "js_a=%s\n", nstsdl_input_translate_event(player[1].ja));
		fprintf(fp, "js_b=%s\n", nstsdl_input_translate_event(player[1].jb));
		fprintf(fp, "js_ta=%s\n", nstsdl_input_translate_event(player[1].jta));
		fprintf(fp, "js_tb=%s\n", nstsdl_input_translate_event(player[1].jtb));
		fprintf(fp, "\n"); // End of Section
		
		fclose(fp);
	}
}

void nstsdl_input_process(Input::Controllers *controllers, SDL_Event event) {
	// Process input events
	switch(event.type) {
		case SDL_KEYUP:
		case SDL_KEYDOWN:
			nstsdl_input_match_keyboard(controllers, event);
			break;
			
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
			nstsdl_input_match_joystick(controllers, event);
			break;

		case SDL_MOUSEBUTTONUP:
		case SDL_MOUSEBUTTONDOWN:
			nstsdl_input_match_mouse(controllers, event);
			break;
			
		default: break;
	}
}

char* nstsdl_input_translate_event(SDL_Event event) {
	// Translate an SDL_Event to an inputcode
	static char inputcode[6];
	
	switch(event.type) {
		case SDL_JOYAXISMOTION:
			sprintf(inputcode, "j%da%d%d", event.jaxis.which, event.jaxis.axis, nstsdl_input_checksign(event.jaxis.value));
			break;
			
		case SDL_JOYHATMOTION:
			sprintf(inputcode, "j%dh%d%d", event.jhat.which, event.jhat.hat, event.jhat.value);
			break;
		
		case SDL_JOYBUTTONUP:	
		case SDL_JOYBUTTONDOWN:
			sprintf(inputcode, "j%db%d", event.jbutton.which, event.jbutton.button);
			break;
	}
	return inputcode;
}

SDL_Event nstsdl_input_translate_string(const char *string) {
	// Translate an inputcode to an SDL_Event
	SDL_Event event;
	
	int type, axis, value;

	int which = 0, whichdigits = 0;

	for (int i = 1; ; i++) {
		if (isdigit(string[i])) {
			whichdigits++;
		}
		else {
			break;
		}
	}

	for (int i = 1; i <= whichdigits; i++) {
		which += (string[i] - '0') * (pow (10, (whichdigits - i)));
	}
	
	if ((unsigned char)string[whichdigits + 1] == 0x61) { // Axis
		axis = string[whichdigits + 2] - '0';
		value = string[whichdigits + 3] - '0';
		event.type = SDL_JOYAXISMOTION;
		event.jaxis.which = which;
		event.jaxis.axis = axis;
		event.jaxis.value = value;
	}
	else if ((unsigned char)string[whichdigits + 1] == 0x62) { // Button
		value = string[whichdigits + 2] - '0';
		if (string[whichdigits + 3]) {
			value = ((string[whichdigits + 2] - '0') * 10) + (string[whichdigits + 3] - '0');
		}
		event.type = SDL_JOYBUTTONDOWN;
		event.jbutton.which = which;
		event.jbutton.button = value;
		
	}
	else if ((unsigned char)string[whichdigits + 1] == 0x68) { // Hat
		axis = string[whichdigits + 2] - '0';
		value = string[whichdigits + 3] - '0';
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

void nstsdl_input_conf(int type, int pnum) {
	// Configure Inputs
	nst_pause();
	
	SDL_Event event, eventbuf;
	int axis = 0, axisnoise = 0, print = 0;
	
	if (type == 0) { // Keyboard
		for (int i = 0; i < 10;) {
			if (print == i) {
				char buttontext[8];
				switch (i) {
					case 0: snprintf(buttontext, sizeof(buttontext), "Up"); break;
					case 1: snprintf(buttontext, sizeof(buttontext), "Down"); break;
					case 2: snprintf(buttontext, sizeof(buttontext), "Left"); break;
					case 3: snprintf(buttontext, sizeof(buttontext), "Right"); break;
					case 4: snprintf(buttontext, sizeof(buttontext), "Select"); break;
					case 5: snprintf(buttontext, sizeof(buttontext), "Start"); break;
					case 6: snprintf(buttontext, sizeof(buttontext), "A"); break;
					case 7: snprintf(buttontext, sizeof(buttontext), "B"); break;
					case 8: snprintf(buttontext, sizeof(buttontext), "Turbo A"); break;
					case 9: snprintf(buttontext, sizeof(buttontext), "Turbo B"); break;
				}
				fprintf(stderr, "Keyboard Input for Player %d - %s\n", pnum + 1, buttontext);
				print++;
			}
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_KEYDOWN) { nstsdl_input_conf_set(event, type, pnum, i); i++; }
			}
		}
	}
	else if (type == 1) { // Joystick
		for (int i = 0; i < 10;) {
			if (print == i) {
				char buttontext[8];
				switch (i) {
					case 0: snprintf(buttontext, sizeof(buttontext), "Up"); break;
					case 1: snprintf(buttontext, sizeof(buttontext), "Down"); break;
					case 2: snprintf(buttontext, sizeof(buttontext), "Left"); break;
					case 3: snprintf(buttontext, sizeof(buttontext), "Right"); break;
					case 4: snprintf(buttontext, sizeof(buttontext), "Select"); break;
					case 5: snprintf(buttontext, sizeof(buttontext), "Start"); break;
					case 6: snprintf(buttontext, sizeof(buttontext), "A"); break;
					case 7: snprintf(buttontext, sizeof(buttontext), "B"); break;
					case 8: snprintf(buttontext, sizeof(buttontext), "Turbo A"); break;
					case 9: snprintf(buttontext, sizeof(buttontext), "Turbo B"); break;
				}
				fprintf(stderr, "Joystick Input for Player %d - %s\n", pnum + 1, buttontext);
				print++;
			}
			
			while (SDL_PollEvent(&event)) {
				if (event.type == SDL_JOYAXISMOTION) {
					if (abs(event.jaxis.value) >= DEADZONE) {
						eventbuf = event;
						axisnoise = 1;
						axis = event.jaxis.axis;
					}
					else if (abs(event.jaxis.value) < DEADZONE && axisnoise && event.jaxis.axis == axis) {
						nstsdl_input_conf_set(eventbuf, type, pnum, i);
						axisnoise = 0;
						i++;
					}
				}
				else if (event.type == SDL_JOYHATMOTION) {
					if (event.jhat.value != SDL_HAT_CENTERED) {
						nstsdl_input_conf_set(event, type, pnum, i);
						i++;
					}
				}
				else if (event.type == SDL_JOYBUTTONDOWN) {
					nstsdl_input_conf_set(event, type, pnum, i);
					i++;
				}
			}
		}
	}
	
	nst_play();
}

void nstsdl_input_conf_button(int pnum, int bnum) {
	// Configure Inputs for single Joystick Buttons
	SDL_Event event, eventbuf;
	int axis = 0, axisnoise = 0, confrunning = 1;
	
	while (confrunning) {
		#ifdef _GTK
		while (gtk_events_pending()) { gtk_main_iteration(); }
		#endif
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_JOYAXISMOTION) {
				if (abs(event.jaxis.value) >= DEADZONE) {
					eventbuf = event;
					axisnoise = 1;
					axis = event.jaxis.axis;
				}
				else if (abs(event.jaxis.value) < DEADZONE && axisnoise && event.jaxis.axis == axis) {
					nstsdl_input_conf_set(eventbuf, 1, pnum, bnum);
					axisnoise = 0;
					confrunning = 0;
				}
			}
			else if (event.type == SDL_JOYHATMOTION) {
				if (event.jhat.value != SDL_HAT_CENTERED) {
					nstsdl_input_conf_set(event, 1, pnum, bnum);
					confrunning = 0;
				}
			}
			else if (event.type == SDL_JOYBUTTONDOWN) {
				nstsdl_input_conf_set(event, 1, pnum, bnum);
				confrunning = 0;
			}
		}
	}
}
