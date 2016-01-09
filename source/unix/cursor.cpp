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
#include <stdlib.h>

#include <SDL.h>

#include "cursor.h"

static SDL_Cursor *cursor;

// Most of this is adapted from the code example in the SDL documentation

static const char *crosshair[] = {
	"              X                 ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"            XX.XX               ",
	"           X.XXX.X              ",
	" XXXXXXXXXXXX   XXXXXXXXXXXX    ",
	"X.X.X.X.X.X.X . X.X.X.X.X.X.X   ",
	" XXXXXXXXXXXX   XXXXXXXXXXXX    ",
	"           X.XXX.X              ",
	"            XX.XX               ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"             XXX                ",
	"             X.X                ",
	"              X                 ",
	"                                ",
	"                                ",
	"                                ",
	"14,14" // Center
};

static SDL_Cursor *cursor_init(const char *image[]) {
	int i, row, col;
	Uint8 data[4*32];
	Uint8 mask[4*32];
	int hot_x, hot_y;
	
	i = -1;
	for (row=0; row<32; ++row) {
		for ( col=0; col<32; ++col ) {
			if ( col % 8 ) {
				data[i] <<= 1;
				mask[i] <<= 1;
			}
			else {
				++i;
				data[i] = mask[i] = 0;
			}
			
			switch (image[row][col]) {
				case 'X':
					data[i] |= 0x01;
					mask[i] |= 0x01;
					break;
				
				case '.':
					mask[i] |= 0x01;
					break;
				
				case ' ': break;
			}
		}
	}
	
	sscanf(image[row], "%d,%d", &hot_x, &hot_y);
	return SDL_CreateCursor(data, mask, 32, 32, hot_x, hot_y);
}

void cursor_set_special(int type) {
	// Set special cursors
	cursor = cursor_init(crosshair);
	SDL_SetCursor(cursor);
}

void cursor_set_default() {
	// Set the cursor to default
	SDL_FreeCursor(cursor);
}
