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

#include <SDL.h>

#include "config.h"

extern settings conf;

int framerate;

int currtick = 0;
int lasttick = 0;

void timing_init() {
	// Initialize timing-related variables
	framerate = conf.timing_speed;
}

bool timing_check() {
	// Check if it's time to execute the next frame
	
	if (conf.timing_vsync && (framerate != conf.timing_altspeed)) {
		return true;
	}
	
	currtick = SDL_GetTicks();
	
	if (currtick > (lasttick + (1000 / framerate))) {
		lasttick = currtick;
		return true;
	}
	
	return false;
}

void timing_set_default() {
	// Set the framerate to the default
	framerate = conf.timing_speed;
	SDL_GL_SetSwapInterval(conf.timing_vsync);
}

void timing_set_altspeed() {
	// Set the framerate to the alternate speed
	framerate = conf.timing_altspeed;
	SDL_GL_SetSwapInterval(0);
}
