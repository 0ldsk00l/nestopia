/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
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
 
#include <SDL.h>
#include <SDL_endian.h>
#include "GL/glu.h"

#include "settings.h"
#include "audio.h"
#include "video.h"
#include "main.h"

using namespace LinuxNst;

extern Settings *sSettings;
extern SDL_Surface *screen;
extern int cur_width, cur_height, cur_Rheight, cur_Rwidth;

bool	using_opengl = false;
bool	linear_filter = false;
GLuint	screenTexID = 0;
int		gl_w, gl_h;
void	*intbuffer;

// init OpenGL and set up for blitting
void opengl_init_structures() {
	
	int scalefactor =  sSettings->GetScaleAmt() + 1;

	glEnable( GL_TEXTURE_2D );

	gl_w = cur_width;
	gl_h = cur_height;

	glGenTextures( 1, &screenTexID ) ;
	glBindTexture( GL_TEXTURE_2D, screenTexID ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear_filter ? GL_LINEAR : GL_NEAREST) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;

	glViewport( 0, 0, screen->w, screen->h);
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_ALPHA_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_3D_EXT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	if (sSettings->GetOverscanMask() == 1) {
		glOrtho(
			0.0,													// Left
			(GLdouble)screen->w,									// Right
			(GLdouble)screen->h - (OVERSCAN_BOTTOM * scalefactor),	// Bottom
			(GLdouble)(OVERSCAN_TOP * scalefactor),					// Top
			-1.0, 1.0
		);
	}
	else {
		glOrtho(0.0, (GLdouble)screen->w, (GLdouble)screen->h, 0.0, -1.0, 1.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// tears down OpenGL when it's no longer needed
void opengl_cleanup() {
	
	if (using_opengl) {
		SDL_FreeSurface( screen );
		glDeleteTextures( 1, &screenTexID );
		
		if (intbuffer) {
			free(intbuffer);
			intbuffer = NULL;
		}
	}
}

// blit the image using OpenGL
void opengl_blit() {

	glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				gl_w, gl_h,
				0,
				GL_BGRA,
				GL_UNSIGNED_BYTE,
		intbuffer);

	glBegin( GL_QUADS ) ;
		glTexCoord2f(1.0f, 1.0f);
		glVertex2i(cur_Rwidth, cur_Rheight);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(cur_Rwidth, 0);
		
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(0, 0);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(0, cur_Rheight);
	glEnd();

	SDL_GL_SwapBuffers();	
}

long Linux_LockScreen(void*& ptr)
{
	if (using_opengl) { // have the engine blit directly to our memory buffer
		ptr = intbuffer;
		return gl_w*4;
	}
	
	else {
		if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

		ptr = intbuffer;
	}
	
	return screen->pitch;
}

void Linux_UnlockScreen(void*) {
	
	if (using_opengl) {
		opengl_blit();
	}
	else {
		unsigned short *src, *dst1;
		unsigned int *srcL, *dst1L;
		int x, y, vdouble;

		// is this a software x2 expand for NTSC mode?
		vdouble = 0;
		if (screen->h == (cur_height<<1)) {
			vdouble = 1;
		}

		if (screen->format->BitsPerPixel == 16) {
			src = (UINT16 *)intbuffer;
			dst1 = (UINT16 *)screen->pixels;

			for (y = 0; y < cur_Rheight; y++) {
				memcpy(dst1, src, cur_width*screen->format->BitsPerPixel/8);
				
				if (vdouble) {
					if (!(y & 1)) {
						src += cur_width;
					}
				}
				
				else {
					src += cur_width;
				}
				dst1 += screen->pitch/2;
			}
		}
		else if (screen->format->BitsPerPixel == 32) {
			srcL = (UINT32 *)intbuffer;
			dst1L = (UINT32 *)screen->pixels;

			for (y = 0; y < cur_Rheight; y++) {
				memcpy(dst1L, srcL, cur_width*screen->format->BitsPerPixel/8);
				
				if (vdouble) {
					if (!(y & 1)) {
						srcL += cur_width;
					}
				}
				
				else {
					srcL += cur_width;
				}
				dst1L += screen->pitch/4;
			}
		}
		else printf("ERROR: Unknown pixel format %d bpp\n", screen->format->BitsPerPixel);

		if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
		SDL_Flip(screen);
	}
}
