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

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"

#include "main.h"
#include "audio.h"
#include "video.h"
#include "config.h"

using namespace Nes::Api;

SDL_Window *sdlwindow;
SDL_Renderer *renderer;
SDL_GLContext glcontext;
SDL_DisplayMode displaymode;

extern settings *conf;
extern int cur_width, cur_height, cur_Rheight, cur_Rwidth;

//extern Video::RenderState renderstate;
extern Video::RenderState::Filter filter;
extern Emulator emulator;

bool	using_opengl = false;
bool	linear_filter = false;
GLuint	screenTexID = 0;
int		gl_w, gl_h;
void	*intbuffer;

// init OpenGL and set up for blitting
void opengl_init_structures() {
	
	int scalefactor = conf->video_scale_factor;

	glEnable( GL_TEXTURE_2D );

	gl_w = cur_width;
	gl_h = cur_height;

	glGenTextures( 1, &screenTexID ) ;
	glBindTexture( GL_TEXTURE_2D, screenTexID ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear_filter ? GL_LINEAR : GL_NEAREST) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
	
	glViewport( 0, 0, cur_Rwidth, cur_Rheight);
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_ALPHA_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_3D_EXT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	if (conf->video_mask_overscan) {
		glOrtho(
			0.0,													// Left
			(GLdouble)cur_Rwidth,									// Right
			(GLdouble)cur_Rheight - (OVERSCAN_BOTTOM * scalefactor),	// Bottom
			(GLdouble)(OVERSCAN_TOP * scalefactor),					// Top
			-1.0, 1.0
		);
	}
	else {
		glOrtho(0.0, (GLdouble)cur_Rwidth, (GLdouble)cur_Rheight, 0.0, -1.0, 1.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// tears down OpenGL when it's no longer needed
void opengl_cleanup() {
	
	if (using_opengl) {
		//SDL_FreeSurface( screen );
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

	SDL_GL_SwapWindow(sdlwindow);
}

void video_init() {
	printf("Initializing video...\n");
}

void video_toggle_filter() {
	conf->video_filter++;
	if (conf->video_filter > 5) { conf->video_filter = 0; }
	Video video(emulator);
	video.ClearFilterUpdateFlag();

	opengl_cleanup();
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
	
	main_init_video();
	SDL_SetWindowSize(sdlwindow, cur_Rwidth, cur_Rheight);
}

void video_toggle_scalefactor() {
	conf->video_scale_factor++;
	if (conf->video_scale_factor > 4) { conf->video_scale_factor = 1; }

	opengl_cleanup();
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
	
	main_init_video();
	SDL_SetWindowSize(sdlwindow, cur_Rwidth, cur_Rheight);
}

void video_create() {

	int displayindex;
	
	Uint32 windowflags = SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE;
	Uint32 renderflags = SDL_RENDERER_ACCELERATED;//|SDL_RENDERER_PRESENTVSYNC;
	
	if(conf->video_fullscreen) {
		SDL_ShowCursor(0);
		windowflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	
	sdlwindow = SDL_CreateWindow(
	"Nestopia",							//    window title
	SDL_WINDOWPOS_UNDEFINED,			//    initial x position
	SDL_WINDOWPOS_UNDEFINED,			//    initial y position
	cur_Rwidth,							//    width, in pixels
	cur_Rheight,						//    height, in pixels
	windowflags);
	
	if(sdlwindow == NULL) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
	}

	displayindex = SDL_GetWindowDisplayIndex(sdlwindow);
	SDL_GetDesktopDisplayMode(displayindex, &displaymode);
	//printf("w: %d\th: %d\n", displaymode.w, displaymode.h);
	//video_resize();
	
	//printf("Window Flags: %x\n", SDL_GetWindowFlags(sdlwindow));
	
	glcontext = SDL_GL_CreateContext(sdlwindow);
	
	if(glcontext == NULL) {
		fprintf(stderr, "Could not create glcontext: %s\n", SDL_GetError());
	}
	
	SDL_GL_MakeCurrent(sdlwindow, glcontext);
	
	renderer = SDL_CreateRenderer(sdlwindow, -1, renderflags);
	
	if(renderer == NULL) {
		fprintf(stderr, "Could not create renderer: %s\n", SDL_GetError());
	}
	
	SDL_GL_SetSwapInterval(1);
}

void video_resize() {
	//SDL_GetWindowSize(sdlwindow, &cur_Rwidth, &cur_Rheight);
	SDL_SetWindowSize(sdlwindow, displaymode.w, displaymode.h);
	cur_Rwidth = displaymode.w;
	cur_Rheight = displaymode.h;
}

void video_toggle_fullscreen() {
	
	Uint32 flags;
	int cursor;
	
	conf->video_fullscreen ^= 1;
	
	if(conf->video_fullscreen) {
		cursor = 0;
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	else { flags = 0; cursor = 1; }
	
	video_set_params();
	main_init_video();
	
	SDL_ShowCursor(cursor);
	SDL_SetWindowFullscreen(sdlwindow, flags);
}

void video_set_filter() {
	// Set the filter
	int scalefactor = conf->video_scale_factor;
	
	switch(conf->video_filter) {
		case 0:	// None (no scaling unless OpenGL)
			filter = Video::RenderState::FILTER_NONE;
			break;

		case 1: // NTSC
			filter = Video::RenderState::FILTER_NTSC;
			break;

		case 2: // scale x
			switch (scalefactor) {
				case 2:
					filter = Video::RenderState::FILTER_SCALE2X;
					break;

				case 3:
					filter = Video::RenderState::FILTER_SCALE3X;
					break;

				default:
					filter = Video::RenderState::FILTER_NONE;
					break;
			}
			break;

		case 3: // scale HQx
			switch (scalefactor) {
				case 2:
					filter = Video::RenderState::FILTER_HQ2X;
					break;

				case 3:
					filter = Video::RenderState::FILTER_HQ3X;
					break;

				case 4:
					filter = Video::RenderState::FILTER_HQ4X;
					break;

				default:
					filter = Video::RenderState::FILTER_NONE;
					break;
			}
			break;
		
		case 4: // 2xSaI
			filter = Video::RenderState::FILTER_2XSAI;
			break;

		case 5: // xBR
			switch (scalefactor) {
				case 2:
					filter = Video::RenderState::FILTER_2XBR;
					break;

				case 3:
					filter = Video::RenderState::FILTER_3XBR;
					break;

				case 4:
					filter = Video::RenderState::FILTER_4XBR;
					break;

				default:
					filter = Video::RenderState::FILTER_NONE;
					break;
			}
		break;
	}
}

void video_set_params() {

	int scalefactor = conf->video_scale_factor;
	
	switch(conf->video_filter)
	{
		case 0:	// None (no scaling unless OpenGL)
			if (conf->video_renderer == 0)
			{
				if (scalefactor > 1)
				{
					//std::cout << "Warning: raw scale factors > 1 not allowed with pure software, use OpenGL\n";
				}
				cur_width = cur_Rwidth = Video::Output::WIDTH;
				cur_height = cur_Rheight = Video::Output::HEIGHT;
			}
			else
			{
				cur_width = Video::Output::WIDTH;
				cur_height = Video::Output::HEIGHT;
				conf->video_tv_aspect == TRUE ? cur_Rwidth = TV_WIDTH * scalefactor : cur_Rwidth = cur_width * scalefactor;
				if (conf->video_mask_overscan) {
					cur_Rheight = (cur_height * scalefactor) - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
				}
				else {
					cur_Rheight = cur_height * scalefactor;
				}
			}

			break;

		case 1: // NTSC
			if (conf->video_renderer == 0)
			{
				if (scalefactor != 2)
				{
					//std::cout << "Warning: NTSC only runs at 2x scale in Software mode.\n";
				}

				scalefactor = 2;
			}

			cur_width = Video::Output::NTSC_WIDTH;
			cur_Rwidth = (cur_width / 2) * scalefactor;
			cur_height = Video::Output::HEIGHT;
			if (conf->video_mask_overscan) {
					cur_Rheight = (cur_height * scalefactor) - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				cur_Rheight = cur_height * scalefactor;
			}
			break;

		case 2: // scale x
			if (scalefactor == 4) 
			{
				//std::cout << "Warning: Scale x only allows scale factors of 3 or less\n";
				scalefactor = 3;	// there is no scale4x
			}

			cur_width = Video::Output::WIDTH * scalefactor;
			cur_height = Video::Output::HEIGHT * scalefactor;
			conf->video_tv_aspect == TRUE ? cur_Rwidth = TV_WIDTH * scalefactor : cur_Rwidth = cur_width;
			if (conf->video_mask_overscan) {
				cur_Rheight = cur_height - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				cur_Rheight = cur_height;
			}
			break;

		case 3: // scale HQx
			cur_width = Video::Output::WIDTH * scalefactor;
			cur_height = Video::Output::HEIGHT * scalefactor;
			conf->video_tv_aspect == TRUE ? cur_Rwidth = TV_WIDTH * scalefactor : cur_Rwidth = cur_width;
			if (conf->video_mask_overscan) {
				cur_Rheight = cur_height - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				cur_Rheight = cur_height;
			}
			break;

		case 4: // 2xSaI
			cur_width = Video::Output::WIDTH * 2;
			cur_height = Video::Output::HEIGHT * 2;
			conf->video_tv_aspect == TRUE ? cur_Rwidth = TV_WIDTH * scalefactor : cur_Rwidth = Video::Output::WIDTH * scalefactor;
			if (conf->video_mask_overscan) {
				cur_Rheight = Video::Output::HEIGHT * scalefactor - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				cur_Rheight = Video::Output::HEIGHT * scalefactor;
			}
			break;

		case 5: // scale xBR
			cur_width = Video::Output::WIDTH * scalefactor;
			cur_height = Video::Output::HEIGHT * scalefactor;
			conf->video_tv_aspect == TRUE ? cur_Rwidth = TV_WIDTH * scalefactor : cur_Rwidth = cur_width;
			if (conf->video_mask_overscan) {
				cur_Rheight = cur_height - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				cur_Rheight = cur_height;
			}
			break;
	}

	if (conf->video_fullscreen && sdlwindow) {
		cur_Rheight = displaymode.h;
		cur_Rwidth = displaymode.w;
	}
	
	//opengl_cleanup();
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
	//intbuffer = malloc(renderstate.bits.count * renderstate.width * renderstate.height);
}

long Linux_LockScreen(void*& ptr)
{
	//if (using_opengl) { // have the engine blit directly to our memory buffer
		ptr = intbuffer;
		return gl_w*4;
	//}
	
	/*else {
		if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

		ptr = intbuffer;
	}
	
	return screen->pitch;*/
}

void Linux_UnlockScreen(void*) {
	
	if (using_opengl) {
		opengl_blit();
	}
	/*else {
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
	}*/
}
