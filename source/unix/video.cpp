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
extern Video::RenderState::Filter filter;
extern Emulator emulator;

bool	using_opengl = false;
bool	linear_filter = false;
GLuint	screenTexID = 0;
int		gl_w, gl_h;
void	*intbuffer;

dimensions rendersize;
dimensions basesize;

void opengl_init_structures() {
	// init OpenGL and set up for blitting
	int scalefactor = conf->video_scale_factor;

	glEnable( GL_TEXTURE_2D );

	gl_w = basesize.w;
	gl_h = basesize.h;

	glGenTextures( 1, &screenTexID ) ;
	glBindTexture( GL_TEXTURE_2D, screenTexID ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear_filter ? GL_LINEAR : GL_NEAREST) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
	
	glViewport( 0, 0, rendersize.w, rendersize.h);
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
			(GLdouble)rendersize.w,									// Right
			(GLdouble)rendersize.h - (OVERSCAN_BOTTOM * scalefactor),	// Bottom
			(GLdouble)(OVERSCAN_TOP * scalefactor),					// Top
			-1.0, 1.0
		);
	}
	else {
		glOrtho(0.0, (GLdouble)rendersize.w, (GLdouble)rendersize.h, 0.0, -1.0, 1.0);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void opengl_cleanup() {
	// tears down OpenGL when it's no longer needed
	if (using_opengl) {
		glDeleteTextures( 1, &screenTexID );
		
		if (intbuffer) {
			free(intbuffer);
			intbuffer = NULL;
		}
	}
}

void opengl_blit() {
	// blit the image using OpenGL
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
		glVertex2i(rendersize.w, rendersize.h);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex2i(rendersize.w, 0);
		
		glTexCoord2f(0.0f, 0.0f);
		glVertex2i(0, 0);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex2i(0, rendersize.h);
	glEnd();

	SDL_GL_SwapWindow(sdlwindow);
}

void video_init() {
	printf("Initializing video...\n");
}

void video_toggle_filter() {
	conf->video_filter++;
	if (conf->video_filter > 5) { conf->video_filter = 0; }
	
	// The scalex filter only allows 3x scale and crashes otherwise
	if (conf->video_filter == 2 && conf->video_scale_factor == 4) {
		conf->video_scale_factor = 3;
	}
	
	Video video(emulator);
	video.ClearFilterUpdateFlag();
	
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
	
	main_init_video();
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void video_toggle_scalefactor() {
	conf->video_scale_factor++;
	if (conf->video_scale_factor > 4) { conf->video_scale_factor = 1; }
	
	// The scalex filter only allows 3x scale and crashes otherwise
	if (conf->video_filter == 2 && conf->video_scale_factor == 4) {
		conf->video_scale_factor = 1;
	}
	
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
	
	main_init_video();
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void video_create() {
	// Create the SDL window
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
	rendersize.w,							//    width, in pixels
	rendersize.h,						//    height, in pixels
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

void video_toggle_fullscreen() {
	// Toggle between fullscreen and window mode
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
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
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
				basesize.w = rendersize.w = Video::Output::WIDTH;
				basesize.h = rendersize.h = Video::Output::HEIGHT;
			}
			else
			{
				basesize.w = Video::Output::WIDTH;
				basesize.h = Video::Output::HEIGHT;
				conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w * scalefactor;
				if (conf->video_mask_overscan) {
					rendersize.h = (basesize.h * scalefactor) - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
				}
				else {
					rendersize.h = basesize.h * scalefactor;
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

			basesize.w = Video::Output::NTSC_WIDTH;
			rendersize.w = (basesize.w / 2) * scalefactor;
			basesize.h = Video::Output::HEIGHT;
			if (conf->video_mask_overscan) {
					rendersize.h = (basesize.h * scalefactor) - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				rendersize.h = basesize.h * scalefactor;
			}
			break;

		case 2: // scale x
			if (scalefactor == 4) {
				fprintf(stderr, "error: ScaleX filter cannot scale to 4x\n");
				scalefactor = 3;
				conf->video_scale_factor = 3;
			}

			basesize.w = Video::Output::WIDTH * scalefactor;
			basesize.h = Video::Output::HEIGHT * scalefactor;
			conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w;
			if (conf->video_mask_overscan) {
				rendersize.h = basesize.h - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				rendersize.h = basesize.h;
			}
			break;

		case 3: // scale HQx
			basesize.w = Video::Output::WIDTH * scalefactor;
			basesize.h = Video::Output::HEIGHT * scalefactor;
			conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w;
			if (conf->video_mask_overscan) {
				rendersize.h = basesize.h - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				rendersize.h = basesize.h;
			}
			break;

		case 4: // 2xSaI
			basesize.w = Video::Output::WIDTH * 2;
			basesize.h = Video::Output::HEIGHT * 2;
			conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = Video::Output::WIDTH * scalefactor;
			if (conf->video_mask_overscan) {
				rendersize.h = Video::Output::HEIGHT * scalefactor - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				rendersize.h = Video::Output::HEIGHT * scalefactor;
			}
			break;

		case 5: // scale xBR
			basesize.w = Video::Output::WIDTH * scalefactor;
			basesize.h = Video::Output::HEIGHT * scalefactor;
			conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w;
			if (conf->video_mask_overscan) {
				rendersize.h = basesize.h - ((OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor);
			}
			else {
				rendersize.h = basesize.h;
			}
			break;
	}

	if (conf->video_fullscreen && sdlwindow) {
		rendersize.h = displaymode.h;
		rendersize.w = displaymode.w;
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
		if (screen->h == (basesize.h<<1)) {
			vdouble = 1;
		}

		if (screen->format->BitsPerPixel == 16) {
			src = (UINT16 *)intbuffer;
			dst1 = (UINT16 *)screen->pixels;

			for (y = 0; y < rendersize.h; y++) {
				memcpy(dst1, src, basesize.w*screen->format->BitsPerPixel/8);
				
				if (vdouble) {
					if (!(y & 1)) {
						src += basesize.w;
					}
				}
				
				else {
					src += basesize.w;
				}
				dst1 += screen->pitch/2;
			}
		}
		else if (screen->format->BitsPerPixel == 32) {
			srcL = (UINT32 *)intbuffer;
			dst1L = (UINT32 *)screen->pixels;

			for (y = 0; y < rendersize.h; y++) {
				memcpy(dst1L, srcL, basesize.w*screen->format->BitsPerPixel/8);
				
				if (vdouble) {
					if (!(y & 1)) {
						srcL += basesize.w;
					}
				}
				
				else {
					srcL += basesize.w;
				}
				dst1L += screen->pitch/4;
			}
		}
		else printf("ERROR: Unknown pixel format %d bpp\n", screen->format->BitsPerPixel);

		if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
		SDL_Flip(screen);
	}*/
}
