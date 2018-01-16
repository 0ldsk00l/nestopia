/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
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

#include "sdlmain.h"
#include "cli.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "config.h"
#include "cheats.h"

// Nst Common
#include "nstcommon.h"

// Nst SDL
#include "sdlvideo.h"

using namespace Nes::Api;

int nst_quit = 0;

extern Input::Controllers *cNstPads;

extern nstpaths_t nstpaths;

extern void (*audio_deinit)();

void nst_schedule_quit() {
	nst_quit = 1;
}

int main(int argc, char *argv[]) {
	// This is the main function
	
	// Set up directories
	nst_set_dirs();
	
	// Set default config options
	config_set_default();
	
	// Read the config file and override defaults
	config_file_read(nstpaths.nstdir);
	
	// Exit if there is no CLI argument
	if (argc == 1) {
		cli_show_usage();
		return 0;
	}
	
	// Handle command line arguments
	cli_handle_command(argc, argv);
	
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	// Detect Joysticks
	input_joysticks_detect();
	
	// Set default input keys
	input_set_default();
	
	// Read the input config file and override defaults
	input_config_read();
	
	// Set audio function pointers
	audio_set_funcs();
	
	// Set the video dimensions
	video_set_dimensions();
	
	// Create the window
	nstsdl_video_create();
	
	// Set up callbacks
	nst_set_callbacks();
	
	// Initialize and load FDS BIOS and NstDatabase.xml
	nst_fds_bios_load();
	nst_db_load();
	
	// Load a rom from the command line
	if (argc > 1) {
		nst_load(argv[argc - 1]);
		/*if (!loaded) {
			fprintf(stderr, "Fatal: Could not load ROM\n");
			exit(1);
		}*/
	}
	
	// Start the main loop
	nst_quit = 0;
	SDL_Event event;
	while (!nst_quit) {
		nst_ogl_render();
		nstsdl_video_swapbuffers();
		
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_QUIT:
					nst_quit = 1;
					break;
				
				case SDL_KEYDOWN:
				case SDL_KEYUP:
				case SDL_JOYHATMOTION:
				case SDL_JOYAXISMOTION:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
				case SDL_MOUSEBUTTONDOWN:
				case SDL_MOUSEBUTTONUP:
					input_process(cNstPads, event);
					break;
				default: break;
			}	
		}
		
		nst_emuloop();
	}
	
	// Remove the cartridge and shut down the NES
	nst_unload();
	
	// Unload the FDS BIOS, NstDatabase.xml, and the custom palette
	nst_db_unload();
	nst_fds_bios_unload();
	nst_palette_unload();
	
	// Deinitialize audio
	audio_deinit();
	
	// Deinitialize joysticks
	input_joysticks_close();
	
	// Write the input config file
	input_config_write();
	
	// Write the config file
	config_file_write(nstpaths.nstdir);

	return 0;
}
