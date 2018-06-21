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

#include <SDL.h>

// Nst Common
#include "nstcommon.h"
#include "config.h"
#include "video.h"
#include "input.h"

// Nst SDL
#include "cursor.h"
#include "sdlvideo.h"

static SDL_GLContext glcontext;
static SDL_Window *sdlwindow;

extern nstpaths_t nstpaths;
extern Emulator emulator;

void nstsdl_video_create() {
	// Create the window
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	
	Uint32 windowflags = SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL;
	
	dimensions_t rendersize = nst_video_get_dimensions_render();
	
	sdlwindow = SDL_CreateWindow(
		NULL,								//    window title
		SDL_WINDOWPOS_UNDEFINED,			//    initial x position
		SDL_WINDOWPOS_UNDEFINED,			//    initial y position
		rendersize.w,						//    width, in pixels
		rendersize.h,						//    height, in pixels
		windowflags);
	
	if(sdlwindow == NULL) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
	}
	
	glcontext = SDL_GL_CreateContext(sdlwindow);
	SDL_GL_MakeCurrent(sdlwindow, glcontext);
	SDL_GL_SetSwapInterval(conf.timing_vsync);
	
	if(glcontext == NULL) {
		fprintf(stderr, "Could not create glcontext: %s\n", SDL_GetError());
	}
	
	fprintf(stderr, "OpenGL: %s\n", glGetString(GL_VERSION));
	
	nst_video_set_dimensions_screen(nstsdl_video_get_dimensions());
	
	// Fullscreen the window after creation
	if (conf.video_fullscreen) { nstsdl_video_toggle_fullscreen(); }
}

void nstsdl_video_destroy() {
	// Destroy the video window
	SDL_DestroyWindow(sdlwindow);
}

dimensions_t nstsdl_video_get_dimensions() {
	// Return the dimensions of the current screen
	dimensions_t scrsize;
	SDL_DisplayMode displaymode;
	int displayindex = SDL_GetWindowDisplayIndex(sdlwindow);
	SDL_GetDesktopDisplayMode(displayindex, &displaymode);
	scrsize.w = displaymode.w;
	scrsize.h = displaymode.h;
	return scrsize;
}

void nstsdl_video_resize() {
	dimensions_t rendersize = nst_video_get_dimensions_render();
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void nstsdl_video_set_cursor() {
	// Set the cursor to what it needs to be
	if (nst_input_zapper_present()) {
		if (conf.misc_disable_cursor_special) {
			SDL_ShowCursor(false);
		}
		else {
			SDL_ShowCursor(true); // Must be set true before being modified if special
			cursor_set_special(Input::ZAPPER);
		}
	}
	else {
		if (conf.misc_disable_cursor || conf.video_fullscreen) { SDL_ShowCursor(false); }
		else { SDL_ShowCursor(true); }
	}
}

void nstsdl_video_set_title(const char *title) {
	// Set the window title
	SDL_SetWindowTitle(sdlwindow, title);
}

void nstsdl_video_swapbuffers() {
	// Swap Buffers
	SDL_GL_SwapWindow(sdlwindow);
}

void nstsdl_video_toggle_fullscreen() {
	
	video_toggle_fullscreen();
	
	Uint32 flags;
	if (conf.video_fullscreen) { flags = SDL_WINDOW_FULLSCREEN_DESKTOP; }
	else { flags = 0; }
	
	SDL_SetWindowFullscreen(sdlwindow, flags);
	
	nstsdl_video_set_cursor();
	
	nst_video_set_dimensions_screen(nstsdl_video_get_dimensions());
	
	video_init();
	
	nstsdl_video_resize();
}

void nstsdl_video_toggle_filter() {
	video_toggle_filter();
	nst_video_set_dimensions_screen(nstsdl_video_get_dimensions());
	video_init();
	nstsdl_video_resize();
}

void nstsdl_video_toggle_scale() {
	video_toggle_scalefactor();
	nst_video_set_dimensions_screen(nstsdl_video_get_dimensions());
	video_init();
	nstsdl_video_resize();
}
