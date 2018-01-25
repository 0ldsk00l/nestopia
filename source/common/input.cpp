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

#include <stdio.h>

#include "nstcommon.h"
#include "config.h"
#include "input.h"
#include "video.h"

static turbo_t turbostate;
static turbo_t turbotoggle;

extern Emulator emulator;

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
