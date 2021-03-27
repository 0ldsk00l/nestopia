/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2016 R. Danbrook
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

#include <cstdio>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Gl_Window.H>

#include <SDL.h>

#include "nstcommon.h"
#include "config.h"
#include "input.h"
#include "video.h"
#include "ini.h"

#include "fltkui.h"

static SDL_Joystick *joystick;
gamepad_t player[NUMGAMEPADS];
inputsettings_t inputconf;
static char inputconfpath[256];

static turbo_t turbostate;
static turbo_t turbotoggle;

extern Emulator emulator;
extern nstpaths_t nstpaths;

extern Emulator emulator;
extern Input::Controllers *cNstPads;

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

void nst_input_init() {
	// Initialize input
	char controller[32];

	for (int i = 0; i < NUMGAMEPADS; i++) {
		Input(emulator).AutoSelectController(i);

		switch(Input(emulator).GetConnectedController(i)) {
			case Input::UNCONNECTED:
				snprintf(controller, sizeof(controller), "%s", "Unconnected");
				break;
			case Input::PAD1:
			case Input::PAD2:
			case Input::PAD3:
			case Input::PAD4:
				snprintf(controller, sizeof(controller), "%s", "Standard Pad");
				break;
			case Input::ZAPPER:
				snprintf(controller, sizeof(controller), "%s", "Zapper");
				break;
			case Input::PADDLE:
				snprintf(controller, sizeof(controller), "%s", "Arkanoid Paddle");
				break;
			case Input::POWERPAD:
				snprintf(controller, sizeof(controller), "%s", "Power Pad");
				break;
			case Input::POWERGLOVE:
				snprintf(controller, sizeof(controller), "%s", "Power Glove");
				break;
			case Input::MOUSE:
				snprintf(controller, sizeof(controller), "%s", "Mouse");
				break;
			case Input::ROB:
				snprintf(controller, sizeof(controller), "%s", "R.O.B.");
				break;
			case Input::FAMILYTRAINER:
				snprintf(controller, sizeof(controller), "%s", "Family Trainer");
				break;
			case Input::FAMILYKEYBOARD:
				snprintf(controller, sizeof(controller), "%s", "Family Keyboard");
				break;
			case Input::SUBORKEYBOARD:
				snprintf(controller, sizeof(controller), "%s", "Subor Keyboard");
				break;
			case Input::DOREMIKKOKEYBOARD:
				snprintf(controller, sizeof(controller), "%s", "Doremikko Keyboard");
				break;
			case Input::HORITRACK:
				snprintf(controller, sizeof(controller), "%s", "Hori Track");
				break;
			case Input::PACHINKO:
				snprintf(controller, sizeof(controller), "%s", "Pachinko");
				break;
			case Input::OEKAKIDSTABLET:
				snprintf(controller, sizeof(controller), "%s", "Oeka Kids Tablet");
				break;
			case Input::KONAMIHYPERSHOT:
				snprintf(controller, sizeof(controller), "%s", "Konami Hypershot");
				break;
			case Input::BANDAIHYPERSHOT:
				snprintf(controller, sizeof(controller), "%s", "Bandai Hypershot");
				break;
			case Input::CRAZYCLIMBER:
				snprintf(controller, sizeof(controller), "%s", "Crazy Climber");
				break;
			case Input::MAHJONG:
				snprintf(controller, sizeof(controller), "%s", "Mahjong");
				break;
			case Input::EXCITINGBOXING:
				snprintf(controller, sizeof(controller), "%s", "Exciting Boxing");
				break;
			case Input::TOPRIDER:
				snprintf(controller, sizeof(controller), "%s", "Top Rider");
				break;
			case Input::POKKUNMOGURAA:
				snprintf(controller, sizeof(controller), "%s", "Pokkun Moguraa");
				break;
			case Input::PARTYTAP:
				snprintf(controller, sizeof(controller), "%s", "PartyTap");
				break;
			case Input::TURBOFILE:
				snprintf(controller, sizeof(controller), "%s", "Turbo File");
				break;
			case Input::BARCODEWORLD:
				snprintf(controller, sizeof(controller), "%s", "Barcode World");
				break;
			default:
				snprintf(controller, sizeof(controller), "%s", "Unknown");
				break;
		}

		fprintf(stderr, "Port %d: %s\n", i + 1, controller);
	}
}

void nst_input_inject(Input::Controllers *controllers, nesinput_t input) {
	// Insert the input signal into the NES
	if(controllers == NULL) { return; }

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

void nst_input_inject_mouse(Input::Controllers *controllers, int b, int s, int x, int y) {
	// Insert input signal for Zappers
	if(controllers == NULL) { return; }

	double xaspect;
	double yaspect;

	if (s) {
		// Get X coords
		if (conf.video_filter == 1) { // NTSC
			xaspect = (double)(Video::Output::WIDTH) / (double)(Video::Output::NTSC_WIDTH / 2);
		}
		else if (conf.video_tv_aspect) {
			xaspect = (double)(Video::Output::WIDTH) / (double)(TV_WIDTH);
		}
		else { xaspect = 1.0; }

		dimensions_t rendersize = nst_video_get_dimensions_render();
		dimensions_t screensize = nst_video_get_dimensions_screen();

		// Calculate fullscreen X coords
		if (conf.video_fullscreen) {
			if (conf.video_stretch_aspect) {
				xaspect = (double)(conf.video_scale_factor * Video::Output::WIDTH) / (double)(screensize.w);
			}
			else {
				// Remove the same amount of pixels as the black area to the left of the screen
				x -= screensize.w / 2.0f - rendersize.w / 2.0f;
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
			yaspect = (double)(conf.video_scale_factor * Video::Output::HEIGHT) / (double)(screensize.h);
			controllers->zapper.y = (y * yaspect) / conf.video_scale_factor;
		}

		// Offscreen
		if (b != 1) { controllers->zapper.x = ~1U; }

		controllers->zapper.fire = true;
	}
	else { controllers->zapper.fire = false; }
}

void nst_input_turbo_init() {
	// Initialize the turbo button states
	turbostate.p1a = turbotoggle.p1a = 0;
	turbostate.p1b = turbotoggle.p1b = 0;
	turbostate.p2a = turbotoggle.p2a = 0;
	turbostate.p2b = turbotoggle.p2b = 0;
}

void nst_input_turbo_pulse(Input::Controllers *controllers) {
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

int nst_input_zapper_present() {
	// Check if a Zapper is presently connected
	if (Input(emulator).GetConnectedController(0) == Input::ZAPPER ||
			Input(emulator).GetConnectedController(1) == Input::ZAPPER) {
			return 1;
	}
	else { return 0; }
}

void nstsdl_input_joysticks_detect() {
	// Initialize any joysticks
	fprintf(stderr, "%i joystick(s) found:\n", SDL_NumJoysticks());

	for (int i = 0; i < SDL_NumJoysticks(); i++) {
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
	SDL_Event reset[2] = { player[0].softreset, player[0].hardreset };

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
			if (event.jbutton.button == reset[0].jbutton.button && event.jbutton.which == reset[0].jbutton.which) { nst_reset(0); }
			if (event.jbutton.button == reset[1].jbutton.button && event.jbutton.which == reset[1].jbutton.which) { nst_reset(1); }
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

	inputconf.qsave1 = FL_F + 5;
	inputconf.qsave2 = FL_F + 6;
	inputconf.qload1 = FL_F + 7;
	inputconf.qload2 = FL_F + 8;
	inputconf.screenshot = FL_F + 9;
	inputconf.fdsflip = FL_F + 3;
	inputconf.fdsswitch = FL_F + 4;
	inputconf.insertcoin1 = FL_F + 1;
	inputconf.insertcoin2 = FL_F + 2;
	inputconf.reset = FL_F + 12;
	inputconf.ffspeed = '`';
	inputconf.rwstart = FL_BackSpace;
	inputconf.rwstop = '\\';
	inputconf.fullscreen = 'f';

	player[0].u = FL_Up;
	player[0].d = FL_Down;
	player[0].l = FL_Left;
	player[0].r = FL_Right;
	player[0].select = FL_Shift_R;
	player[0].start = FL_Enter;
	player[0].a = 'z';
	player[0].b = 'a';
	player[0].ta = 'x';
	player[0].tb = 's';

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
	player[0].softreset = nstsdl_input_translate_string("j0b99");
	player[0].hardreset = nstsdl_input_translate_string("j0b99");

	player[1].u = 'i';
	player[1].d = 'j';
	player[1].l = 'k';
	player[1].r = 'l';
	player[1].select = FL_Shift_L;
	player[1].start = FL_Control_L;
	player[1].a = 'm';
	player[1].b = 'n';
	player[1].ta = 'b';
	player[1].tb = 'v';

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

void fltkui_input_conf_set(int kval, int pnum, int bnum) {
	// Set an input item to what was requested by configuration process
	switch (bnum) {
		case 0: player[pnum].u = kval; break;
		case 1: player[pnum].d = kval; break; 
		case 2: player[pnum].l = kval; break;
		case 3: player[pnum].r = kval; break;
		case 4: player[pnum].select = kval; break;
		case 5: player[pnum].start = kval; break;
		case 6: player[pnum].a = kval; break;
		case 7: player[pnum].b = kval; break;
		case 8: player[pnum].ta = kval; break;
		case 9: player[pnum].tb = kval; break;
		default: break;
	}
}

void nstsdl_input_conf_set(SDL_Event event, int pnum, int bnum) {
	// Set an input item to what was requested by configuration process
	switch (bnum) {
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

static int nstsdl_input_config_match(void* user, const char* section, const char* name, const char* value) {
	// Match values from input config file and populate live config
	inputsettings_t* pconfig = (inputsettings_t*)user;

	// User Interface
	if (MATCH("ui", "qsave1")) { pconfig->qsave1 = atoi(value); }
	else if (MATCH("ui", "qsave2")) { pconfig->qsave2 = atoi(value); }
	else if (MATCH("ui", "qload1")) { pconfig->qload1 = atoi(value); }
	else if (MATCH("ui", "qload2")) { pconfig->qload2 = atoi(value); }

	else if (MATCH("ui", "screenshot")) { pconfig->screenshot = atoi(value); }

	else if (MATCH("ui", "fdsflip")) { pconfig->fdsflip = atoi(value); }
	else if (MATCH("ui", "fdsswitch")) { pconfig->fdsswitch = atoi(value); }

	else if (MATCH("ui", "insertcoin1")) { pconfig->insertcoin1 = atoi(value); }
	else if (MATCH("ui", "insertcoin2")) { pconfig->insertcoin2 = atoi(value); }

	else if (MATCH("ui", "reset")) { pconfig->reset = atoi(value); }

	else if (MATCH("ui", "ffspeed")) { pconfig->ffspeed = atoi(value); }
	else if (MATCH("ui", "rwstart")) { pconfig->rwstart = atoi(value); }
	else if (MATCH("ui", "rwstop")) { pconfig->rwstop = atoi(value); }

	else if (MATCH("ui", "fullscreen")) { pconfig->fullscreen = atoi(value); }

	// Player 1
	else if (MATCH("gamepad1", "kb_u")) { pconfig->kb_p1u = atoi(value); }
	else if (MATCH("gamepad1", "kb_d")) { pconfig->kb_p1d = atoi(value); }
	else if (MATCH("gamepad1", "kb_l")) { pconfig->kb_p1l = atoi(value); }
	else if (MATCH("gamepad1", "kb_r")) { pconfig->kb_p1r = atoi(value); }
	else if (MATCH("gamepad1", "kb_select")) { pconfig->kb_p1select = atoi(value); }
	else if (MATCH("gamepad1", "kb_start")) { pconfig->kb_p1start = atoi(value); }
	else if (MATCH("gamepad1", "kb_a")) { pconfig->kb_p1a = atoi(value); }
	else if (MATCH("gamepad1", "kb_b")) { pconfig->kb_p1b = atoi(value); }
	else if (MATCH("gamepad1", "kb_ta")) { pconfig->kb_p1ta = atoi(value); }
	else if (MATCH("gamepad1", "kb_tb")) { pconfig->kb_p1tb = atoi(value); }

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

	else if (MATCH("gamepad1", "js_softreset")) { pconfig->js_softreset = strdup(value); }
	else if (MATCH("gamepad1", "js_hardreset")) { pconfig->js_hardreset = strdup(value); }

	// Player 2
	else if (MATCH("gamepad2", "kb_u")) { pconfig->kb_p2u = atoi(value); }
	else if (MATCH("gamepad2", "kb_d")) { pconfig->kb_p2d = atoi(value); }
	else if (MATCH("gamepad2", "kb_l")) { pconfig->kb_p2l = atoi(value); }
	else if (MATCH("gamepad2", "kb_r")) { pconfig->kb_p2r = atoi(value); }
	else if (MATCH("gamepad2", "kb_select")) { pconfig->kb_p2select = atoi(value); }
	else if (MATCH("gamepad2", "kb_start")) { pconfig->kb_p2start = atoi(value); }
	else if (MATCH("gamepad2", "kb_a")) { pconfig->kb_p2a = atoi(value); }
	else if (MATCH("gamepad2", "kb_b")) { pconfig->kb_p2b = atoi(value); }
	else if (MATCH("gamepad2", "kb_ta")) { pconfig->kb_p2ta = atoi(value); }
	else if (MATCH("gamepad2", "kb_tb")) { pconfig->kb_p2tb = atoi(value); }

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
	snprintf(inputconfpath, sizeof(inputconfpath), "%sinput.conf", nstpaths.nstconfdir);

	if (ini_parse(inputconfpath, nstsdl_input_config_match, &inputconf) < 0) {
		fprintf(stderr, "Failed to load input config file %s: Using defaults.\n", inputconfpath);
	}
	else { // Map the input settings from the config file
		player[0].u = inputconf.kb_p1u;
		player[0].d = inputconf.kb_p1d;
		player[0].l = inputconf.kb_p1l;
		player[0].r = inputconf.kb_p1r;
		player[0].select = inputconf.kb_p1select;
		player[0].start = inputconf.kb_p1start;
		player[0].a = inputconf.kb_p1a;
		player[0].b = inputconf.kb_p1b;
		player[0].ta = inputconf.kb_p1ta;
		player[0].tb = inputconf.kb_p1tb;

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

		if (inputconf.js_softreset) { player[0].softreset = nstsdl_input_translate_string(inputconf.js_softreset); }
		if (inputconf.js_hardreset) { player[0].hardreset = nstsdl_input_translate_string(inputconf.js_hardreset); }

		// Player 2
		player[1].u = inputconf.kb_p2u;
		player[1].d = inputconf.kb_p2d;
		player[1].l = inputconf.kb_p2l;
		player[1].r = inputconf.kb_p2r;
		player[1].select = inputconf.kb_p2select;
		player[1].start = inputconf.kb_p2start;
		player[1].a = inputconf.kb_p2a;
		player[1].b = inputconf.kb_p2b;
		player[1].ta = inputconf.kb_p2ta;
		player[1].tb = inputconf.kb_p2tb;

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
		fprintf(fp, "; Nestopia UE Input Configuration File\n\n");
		fprintf(fp, "; Possible values for joystick input:\n; j[joystick number][a|b|h][button/hat/axis number][1/0 = +/- (axes only)]\n");
		fprintf(fp, "; Example: j0b3 = joystick 0, button 3. j1a11 = joystick 1, axis 1 +\n\n");

		fprintf(fp, "[ui]\n");
		fprintf(fp, "qsave1=%d\n", inputconf.qsave1);
		fprintf(fp, "qsave2=%d\n", inputconf.qsave2);
		fprintf(fp, "qload1=%d\n", inputconf.qload1);
		fprintf(fp, "qload2=%d\n", inputconf.qload2);
		fprintf(fp, "screenshot=%d\n", inputconf.screenshot);
		fprintf(fp, "fdsflip=%d\n", inputconf.fdsflip);
		fprintf(fp, "fdsswitch=%d\n", inputconf.fdsswitch);
		fprintf(fp, "insertcoin1=%d\n", inputconf.insertcoin1);
		fprintf(fp, "insertcoin2=%d\n", inputconf.insertcoin2);
		fprintf(fp, "reset=%d\n", inputconf.reset);
		fprintf(fp, "ffspeed=%d\n", inputconf.ffspeed);
		fprintf(fp, "rwstart=%d\n", inputconf.rwstart);
		fprintf(fp, "rwstop=%d\n", inputconf.rwstop);
		fprintf(fp, "fullscreen=%d\n", inputconf.fullscreen);
		fprintf(fp, "\n"); // End of Section

		fprintf(fp, "[gamepad1]\n");
		fprintf(fp, "kb_u=%d\n", player[0].u);
		fprintf(fp, "kb_d=%d\n", player[0].d);
		fprintf(fp, "kb_l=%d\n", player[0].l);
		fprintf(fp, "kb_r=%d\n", player[0].r);
		fprintf(fp, "kb_select=%d\n", player[0].select);
		fprintf(fp, "kb_start=%d\n", player[0].start);
		fprintf(fp, "kb_a=%d\n", player[0].a);
		fprintf(fp, "kb_b=%d\n", player[0].b);
		fprintf(fp, "kb_ta=%d\n", player[0].ta);
		fprintf(fp, "kb_tb=%d\n", player[0].tb);

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
		fprintf(fp, "js_softreset=%s\n", nstsdl_input_translate_event(player[0].softreset));
		fprintf(fp, "js_hardreset=%s\n", nstsdl_input_translate_event(player[0].hardreset));
		fprintf(fp, "\n"); // End of Section

		fprintf(fp, "[gamepad2]\n");
		fprintf(fp, "kb_u=%d\n", player[1].u);
		fprintf(fp, "kb_d=%d\n", player[1].d);
		fprintf(fp, "kb_l=%d\n", player[1].l);
		fprintf(fp, "kb_r=%d\n", player[1].r);
		fprintf(fp, "kb_select=%d\n", player[1].select);
		fprintf(fp, "kb_start=%d\n", player[1].start);
		fprintf(fp, "kb_a=%d\n", player[1].a);
		fprintf(fp, "kb_b=%d\n", player[1].b);
		fprintf(fp, "kb_ta=%d\n", player[1].ta);
		fprintf(fp, "kb_tb=%d\n", player[1].tb);

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

		fclose(fp);
	}
}

void nstsdl_input_process(Input::Controllers *controllers, SDL_Event event) {
	// Process input events
	switch(event.type) {
		case SDL_JOYBUTTONUP:
		case SDL_JOYBUTTONDOWN:
		case SDL_JOYAXISMOTION:
		case SDL_JOYHATMOTION:
			nstsdl_input_match_joystick(controllers, event);
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

void nstsdl_input_conf_button(int pnum, int bnum) {
	// Configure Inputs for single Joystick Buttons
	SDL_Event event, eventbuf;
	int axis = 0, axisnoise = 0, iterations = 0, confrunning = 1;

	if (SDL_NumJoysticks() == 0) { return; }

	while (confrunning) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_JOYAXISMOTION) {
				if (abs(event.jaxis.value) >= DEADZONE) {
					eventbuf = event;
					axisnoise = 1;
					axis = event.jaxis.axis;
				}
				else if (abs(event.jaxis.value) < DEADZONE && axisnoise && event.jaxis.axis == axis) {
					nstsdl_input_conf_set(eventbuf, pnum, bnum);
					axisnoise = 0;
					confrunning = 0;
				}
			}
			else if (event.type == SDL_JOYHATMOTION) {
				if (event.jhat.value != SDL_HAT_CENTERED) {
					nstsdl_input_conf_set(event, pnum, bnum);
					confrunning = 0;
				}
			}
			else if (event.type == SDL_JOYBUTTONDOWN) {
				nstsdl_input_conf_set(event, pnum, bnum);
				confrunning = 0;
			}
		}
		if (++iterations > 3000) { break; } // Roughly 3 second timeout
		SDL_Delay(1);
	}
}

void fltkui_input_process_key(int e) {
	nesinput_t input;

	input.nescode = input.player = input.pressed = input.turboa = input.turbob = 0;

	if (e == FL_KEYDOWN) {
		input.pressed = 1;
		if (Fl::event_key() == '`') nst_timing_set_ffspeed();
		else if (Fl::event_key() == inputconf.qsave1) nst_state_quicksave(0);
		else if (Fl::event_key() == inputconf.qsave2) nst_state_quicksave(1);
		else if (Fl::event_key() == inputconf.qload1) nst_state_quickload(0);
		else if (Fl::event_key() == inputconf.qload2) nst_state_quickload(1);
		else if (Fl::event_key() == inputconf.screenshot) { video_screenshot(NULL); }
		else if (Fl::event_key() == inputconf.fdsflip) { nst_fds_flip(); }
		else if (Fl::event_key() == inputconf.fdsswitch) { nst_fds_switch(); }
		else if (Fl::event_key() == inputconf.insertcoin1) { cNstPads->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_1; }
		else if (Fl::event_key() == inputconf.insertcoin2) { cNstPads->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_2; }
		else if (Fl::event_key() == inputconf.reset) { nst_reset(0); }
		else if (Fl::event_key() == inputconf.rwstart) { nst_set_rewind(0); }
		else if (Fl::event_key() == inputconf.rwstop) { nst_set_rewind(1); }
		else if (Fl::event_key() == ' ') { cNstPads->pad[1].mic = 0x04; }
	}
	else {
		if (Fl::event_key() == inputconf.ffspeed) nst_timing_set_default();
		else if (Fl::event_key() == inputconf.fullscreen) fltkui_fullscreen(NULL, NULL);
		else if (Fl::event_key() == ' ') { cNstPads->pad[1].mic = 0x00; }
	}

	for (int i = 0; i < NUMGAMEPADS; i++) {
		if (Fl::event_key() == player[i].u) { input.player = i; input.nescode = Input::Controllers::Pad::UP; }
		else if (Fl::event_key() == player[i].d) { input.player = i; input.nescode = Input::Controllers::Pad::DOWN; }
		else if (Fl::event_key() == player[i].l) { input.player = i; input.nescode = Input::Controllers::Pad::LEFT; }
		else if (Fl::event_key() == player[i].r) { input.player = i; input.nescode = Input::Controllers::Pad::RIGHT; }
		else if (Fl::event_key() == player[i].select) { input.player = i; input.nescode = Input::Controllers::Pad::SELECT; }
		else if (Fl::event_key() == player[i].start) { input.player = i; input.nescode = Input::Controllers::Pad::START; }
		else if (Fl::event_key() == player[i].a) { input.player = i; input.nescode = Input::Controllers::Pad::A; }
		else if (Fl::event_key() == player[i].b) { input.player = i; input.nescode = Input::Controllers::Pad::B; }
		else if (Fl::event_key() == player[i].ta) { input.player = i; input.turboa = 1; input.nescode = Input::Controllers::Pad::A; }
		else if (Fl::event_key() == player[i].tb) { input.player = i; input.turbob = 1; input.nescode = Input::Controllers::Pad::B; }
	}

	nst_input_inject(cNstPads, input);
}
