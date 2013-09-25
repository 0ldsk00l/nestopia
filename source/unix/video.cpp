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

#include <stdio.h>
#include <stdlib.h>

#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_endian.h>
#include "GL/glu.h"

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"

#include "main.h"
#include "audio.h"
#include "video.h"
#include "config.h"

using namespace Nes::Api;

bool	linear_filter = false;
GLuint	screenTexID = 0;
int		gl_w, gl_h;
void	*intbuffer;

SDL_Window *sdlwindow;
SDL_GLContext glcontext;
SDL_DisplayMode displaymode;

Video::RenderState::Filter filter;
Video::RenderState renderstate;

dimensions rendersize;
dimensions basesize;

extern settings *conf;
extern Emulator emulator;

void opengl_init_structures() {
	// init OpenGL and set up for blitting
	int scalefactor = conf->video_scale_factor;

	// Fix the fencepost issue when masking overscan
	float fencepost = scalefactor / 2.0;
	
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
/*Left*/	0.0,
/*Right*/   (GLdouble)rendersize.w,
/*Bottom*/	(GLdouble)rendersize.h - (OVERSCAN_BOTTOM * scalefactor) + fencepost,
/*Top*/		(GLdouble)(OVERSCAN_TOP * scalefactor) - fencepost,
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
	glDeleteTextures( 1, &screenTexID );
	
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
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
	// Initialize video structures	
	int scalefactor = conf->video_scale_factor;
	
	video_set_params();
	video_set_filter();
	
	linear_filter = (conf->video_renderer == 2);
	
	opengl_init_structures();
	
	// Set up the render state parameters
	renderstate.filter = filter;
	renderstate.width = basesize.w;
	renderstate.height = basesize.h;
	renderstate.bits.count = 32;
	
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
	renderstate.bits.mask.r = 0x000000ff;
	renderstate.bits.mask.g = 0xff000000;
	renderstate.bits.mask.b = 0x00ff0000;
#else
	renderstate.bits.mask.r = 0x00ff0000;
	renderstate.bits.mask.g = 0x0000ff00;
	renderstate.bits.mask.b = 0x000000ff;
#endif
	
	// allocate the intermediate render buffer
	intbuffer = malloc(renderstate.bits.count * renderstate.width * renderstate.height);
	
	// acquire the video interface
	Video video(emulator);
	
	// set the sprite limit
	video.EnableUnlimSprites(conf->video_unlimited_sprites ? false : true);
	
	// Set NTSC options
	switch (conf->video_ntsc_mode) {
		case 0:	// Composite
			video.SetSharpness(Video::DEFAULT_SHARPNESS_COMP);
			video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_COMP);
			video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_COMP);
			video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_COMP);
			video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_COMP);
			break;

		case 1:	// S-Video
			video.SetSharpness(Video::DEFAULT_SHARPNESS_SVIDEO);
			video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_SVIDEO);
			video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_SVIDEO);
			video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO);
			video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_SVIDEO);
			break;

		case 2:	// RGB
			video.SetSharpness(Video::DEFAULT_SHARPNESS_RGB);
			video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_RGB);
			video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_RGB);
			video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_RGB);
			video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_RGB);
			break;
	}
	
	// Set xBR options
	if (conf->video_filter == 2) {
		video.SetCornerRounding(conf->video_xbr_corner_rounding);
		video.SetBlend(conf->video_xbr_pixel_blending);
	}
	
	video.ClearFilterUpdateFlag();
	
	// set the render state
	if (NES_FAILED(video.SetRenderState(renderstate))) {
		fprintf(stderr, "Nestopia core rejected render state\n");
		exit(1);
	}
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
	video_init();
	
	SDL_ShowCursor(cursor);
	SDL_SetWindowFullscreen(sdlwindow, flags);
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void video_toggle_filter() {
	conf->video_filter++;
	
	// Intentionally leaving out scalex
	if (conf->video_filter > 4) { conf->video_filter = 0; }
	
	// The scalex filter only allows 3x scale and crashes otherwise
	if (conf->video_filter == 5 && conf->video_scale_factor == 4) {
		conf->video_scale_factor = 3;
	}
	
	Video video(emulator);
	video.ClearFilterUpdateFlag();
	
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
	
	video_init();
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void video_toggle_scalefactor() {
	conf->video_scale_factor++;
	if (conf->video_scale_factor > 4) { conf->video_scale_factor = 1; }
	
	// The scalex filter only allows 3x scale and crashes otherwise
	if (conf->video_filter == 5 && conf->video_scale_factor == 4) {
		conf->video_scale_factor = 1;
	}
	
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
	
	video_init();
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void video_create() {
	// Create the SDL window
	int displayindex;
	
	Uint32 windowflags = SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE;
	
	if(conf->video_fullscreen) {
		SDL_ShowCursor(0);
		windowflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	
	sdlwindow = SDL_CreateWindow(
	"Nestopia",							//    window title
	SDL_WINDOWPOS_UNDEFINED,			//    initial x position
	SDL_WINDOWPOS_UNDEFINED,			//    initial y position
	rendersize.w,						//    width, in pixels
	rendersize.h,						//    height, in pixels
	windowflags);
	
	if(sdlwindow == NULL) {
		fprintf(stderr, "Could not create window: %s\n", SDL_GetError());
	}

	displayindex = SDL_GetWindowDisplayIndex(sdlwindow);
	SDL_GetDesktopDisplayMode(displayindex, &displaymode);
	//printf("w: %d\th: %d\n", displaymode.w, displaymode.h);
	
	//printf("Window Flags: %x\n", SDL_GetWindowFlags(sdlwindow));
	
	glcontext = SDL_GL_CreateContext(sdlwindow);
	
	if(glcontext == NULL) {
		fprintf(stderr, "Could not create glcontext: %s\n", SDL_GetError());
	}
	
	SDL_GL_MakeCurrent(sdlwindow, glcontext);
	
	SDL_GL_SetSwapInterval(1);
}

void video_set_filter() {
	// Set the filter
	int scalefactor = conf->video_scale_factor;
	
	switch(conf->video_filter) {
		case 0:	// None
			filter = Video::RenderState::FILTER_NONE;
			break;

		case 1: // NTSC
			filter = Video::RenderState::FILTER_NTSC;
			break;

		case 2: // xBR
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

		case 5: // scale x
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
		break;
	}
}

void video_set_params() {

	int scalefactor = conf->video_scale_factor;
	
	switch(conf->video_filter)
	{
		case 0:	// None
			basesize.w = Video::Output::WIDTH;
			basesize.h = Video::Output::HEIGHT;
			conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w * scalefactor;
			rendersize.h = basesize.h * scalefactor;
			break;

		case 1: // NTSC
			basesize.w = Video::Output::NTSC_WIDTH;
			rendersize.w = (basesize.w / 2) * scalefactor;
			basesize.h = Video::Output::HEIGHT;
			rendersize.h = basesize.h * scalefactor;
			break;

		case 2: // xBR
		case 3: // HqX
		case 5: // ScaleX
			if (conf->video_filter == 5 && scalefactor == 4) {
				fprintf(stderr, "error: ScaleX filter cannot scale to 4x\n");
				conf->video_scale_factor = scalefactor = 3;
			}
			
			basesize.w = Video::Output::WIDTH * scalefactor;
			basesize.h = Video::Output::HEIGHT * scalefactor;
			conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w;
			rendersize.h = basesize.h;
			break;
		
		case 4: // 2xSaI
			basesize.w = Video::Output::WIDTH * 2;
			basesize.h = Video::Output::HEIGHT * 2;
			conf->video_tv_aspect == TRUE ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = Video::Output::WIDTH * scalefactor;
			rendersize.h = Video::Output::HEIGHT * scalefactor;
			break;
	}

	if (conf->video_mask_overscan) {
		rendersize.h -= (OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor;
	}
	
	// Calculate the aspect from the height because it's smaller
	float aspect = (float)displaymode.h / (float)rendersize.h;
	
	if (conf->video_preserve_aspect && conf->video_fullscreen && sdlwindow) {
		rendersize.h *= aspect;
		rendersize.w *= aspect;
	}
	else if (conf->video_fullscreen && sdlwindow) {
		rendersize.h = displaymode.h;
		rendersize.w = displaymode.w;
	}
	
	if (intbuffer) {
		free(intbuffer);
		intbuffer = NULL;
	}
}

long Linux_LockScreen(void*& ptr) {
	ptr = intbuffer;
	return gl_w*4;
}

void Linux_UnlockScreen(void*) {
	opengl_blit();
}
