/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
 * Copyright (C) 2012-2014 R. Danbrook
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

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiInput.hpp"
#include "core/api/NstApiVideo.hpp"

#include "main.h"
#include "audio.h"
#include "video.h"
#include "config.h"
#include "cursor.h"

#ifdef _GTK
#include "gtkui/gtkui.h"
extern GtkWidget *drawingarea;
#endif

using namespace Nes::Api;

GLuint screenTexID = 0;
void *videobuf;

SDL_Window *sdlwindow;
SDL_Window *embedwindow;
SDL_GLContext glcontext;
SDL_DisplayMode displaymode;

Video::RenderState::Filter filter;
Video::RenderState renderstate;

dimensions_t basesize, rendersize;

extern settings_t conf;
extern nstpaths_t nstpaths;
extern Emulator emulator;

void opengl_init_structures() {
	// init OpenGL and set up for blitting
	int scalefactor = conf.video_scale_factor;
	
	// Fix the fencepost issue when masking overscan
	float fencepost = scalefactor / 2.0;
	
	glEnable( GL_TEXTURE_2D );
	
	glGenTextures( 1, &screenTexID ) ;
	glBindTexture( GL_TEXTURE_2D, screenTexID ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, conf.video_linear_filter ? GL_LINEAR : GL_NEAREST) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;
	
	conf.video_fullscreen ? 
	glViewport( displaymode.w / 2.0f - rendersize.w / 2.0f, 0, rendersize.w, rendersize.h) :
	glViewport( 0, 0, rendersize.w, rendersize.h);
	
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_ALPHA_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_3D_EXT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	
	if (conf.video_unmask_overscan) {
		glOrtho(
			conf.video_linear_filter ? 2.0 : 0.0,
			(GLdouble)rendersize.w,
			(GLdouble)rendersize.h,
			0.0,
			-1.0, 1.0
		);
	}
	else {
		glOrtho(
			conf.video_linear_filter ? 2.0 : 0.0,
			(GLdouble)rendersize.w,
			(GLdouble)rendersize.h - (OVERSCAN_BOTTOM * scalefactor) + fencepost,
			(GLdouble)(OVERSCAN_TOP * scalefactor) - fencepost,
			-1.0, 1.0
		);
	}
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void opengl_cleanup() {
	// tears down OpenGL when it's no longer needed
	if (screenTexID) { glDeleteTextures( 1, &screenTexID ); }
	
	if (videobuf) {
		free(videobuf);
		videobuf = NULL;
	}
}

void opengl_blit() {
	// blit the image using OpenGL
	glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				basesize.w, basesize.h,
				0,
				GL_BGRA,
				GL_UNSIGNED_BYTE,
		videobuf);

	glBegin( GL_QUADS ) ;
		glTexCoord2f(1.0f, 1.0f);
		glVertex2f(rendersize.w, rendersize.h);
		
		glTexCoord2f(1.0f, 0.0f);
		glVertex2f(rendersize.w, 0);
		
		glTexCoord2f(0.0f, 0.0f);
		glVertex2f(0, 0);
		
		glTexCoord2f(0.0f, 1.0f);
		glVertex2f(0, rendersize.h);
	glEnd();
	
	#ifdef _GTK
	if (conf.misc_disable_gui) { SDL_GL_SwapWindow(sdlwindow); }
	else { conf.video_fullscreen ? SDL_GL_SwapWindow(sdlwindow) : SDL_GL_SwapWindow(embedwindow); }
	#else
	SDL_GL_SwapWindow(sdlwindow);
	#endif
}

void video_init() {
	// Initialize video
	opengl_cleanup();
	
	video_set_dimensions();
	video_set_filter();
	
	// Allocate the video buffer
	videobuf = malloc(renderstate.bits.count * renderstate.width * renderstate.height);
	
	opengl_init_structures();
	
	video_set_cursor();
}

void video_toggle_fullscreen() {
	// Toggle between fullscreen and window mode
	Uint32 flags;
	conf.video_fullscreen ^= 1;
	
	if (conf.video_fullscreen) {
		flags = SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	else { flags = 0; }
	
	#ifdef _GTK
	if (conf.video_fullscreen) {
		SDL_DestroyWindow(sdlwindow);
		video_create_standalone();
		SDL_GL_MakeCurrent(sdlwindow, glcontext);
	}
	else {
		if (!conf.misc_disable_gui) {
			SDL_GL_MakeCurrent(embedwindow, glcontext);
			SDL_DestroyWindow(sdlwindow);
			gtkui_resize();
		}
		else {
			video_set_dimensions();
			SDL_SetWindowFullscreen(sdlwindow, flags);
			SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
		}
	}
	#else
	SDL_SetWindowFullscreen(sdlwindow, flags);
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
	#endif
	
	video_set_cursor();
	video_init();
}

void video_toggle_filter() {
	conf.video_filter++;
	
	// Intentionally leaving out scalex
	if (conf.video_filter > 4) { conf.video_filter = 0; }
	
	// The scalex filter only allows 3x scale and crashes otherwise
	if (conf.video_filter == 5 && conf.video_scale_factor == 4) {
		conf.video_scale_factor = 3;
	}
	
	video_init();
}

void video_toggle_filterupdate() {
	// Clear the filter update flag
	Video video(emulator);
	video.ClearFilterUpdateFlag();
}

void video_toggle_scalefactor() {
	// Toggle video scale factor
	conf.video_scale_factor++;
	if (conf.video_scale_factor > 4) { conf.video_scale_factor = 1; }
	
	// The scalex filter only allows 3x scale and crashes otherwise
	if (conf.video_filter == 5 && conf.video_scale_factor == 4) {
		conf.video_scale_factor = 1;
	}
	
	video_init();
}

void video_create_standalone() {
	// Create a standalone SDL window
	int displayindex;
	
	Uint32 windowflags = SDL_WINDOW_SHOWN|SDL_WINDOW_OPENGL|SDL_WINDOW_RESIZABLE;
	
	if (conf.video_fullscreen) {
		SDL_ShowCursor(0);
		windowflags |= SDL_WINDOW_FULLSCREEN_DESKTOP;
	}
	
	sdlwindow = SDL_CreateWindow(
		nstpaths.gamename,					//    window title
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
	
	SDL_GL_MakeCurrent(sdlwindow, glcontext);
	SDL_GL_SetSwapInterval(conf.timing_vsync);
}

void video_create_embedded() {
	// Create an embedded SDL window
	#ifdef _GTK
	GdkWindow *gdkwindow;
	gdkwindow = gtk_widget_get_window(drawingarea);
	embedwindow = SDL_CreateWindowFrom((void *)GDK_WINDOW_XID(gtk_widget_get_window(drawingarea)));
	embedwindow->flags |= SDL_WINDOW_OPENGL;
	SDL_GL_LoadLibrary(NULL);
	#endif
}

void video_create() {
	// Create the necessary window(s)
	
	#ifdef _GTK
	if (!conf.misc_disable_gui) {
		video_create_embedded();
		glcontext = SDL_GL_CreateContext(embedwindow);
	}
	else {
		video_create_standalone();
		glcontext = SDL_GL_CreateContext(sdlwindow);
	}
	
	if (conf.video_fullscreen) {
		video_create_standalone();
		glcontext = SDL_GL_CreateContext(sdlwindow);
	}
	#else
	video_create_standalone();
	glcontext = SDL_GL_CreateContext(sdlwindow);
	#endif
	
	if(glcontext == NULL) {
		fprintf(stderr, "Could not create glcontext: %s\n", SDL_GetError());
	}
}

void video_set_filter() {
	// Set the filter
	Video video(emulator);
	int scalefactor = conf.video_scale_factor;
	
	switch(conf.video_filter) {
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
	
	// Set the sprite limit:  false = enable sprite limit, true = disable sprite limit
	video.EnableUnlimSprites(conf.video_unlimited_sprites ? true : false);
	
	// Set Palette options
	switch (conf.video_palette_mode) {
		case 0: // YUV
			video.GetPalette().SetMode(Video::Palette::MODE_YUV);
			break;
		
		case 1: // RGB
			video.GetPalette().SetMode(Video::Palette::MODE_RGB);
	}
	
	// Set YUV Decoder/Picture options
	if (video.GetPalette().GetMode() != Video::Palette::MODE_RGB) {
		switch (conf.video_decoder) {
			case 0: // Consumer
				video.SetDecoder(Video::DECODER_CONSUMER);
				break;
			
			case 1: // Canonical
				video.SetDecoder(Video::DECODER_CANONICAL);
				break;
			
			case 2: // Alternative (Canonical with yellow boost)
				video.SetDecoder(Video::DECODER_ALTERNATIVE);
				break;
			
			default: break;
		}
		
		video.SetBrightness(conf.video_brightness);
		video.SetSaturation(conf.video_saturation);
		video.SetContrast(conf.video_contrast);
		video.SetHue(conf.video_hue);
	}
	
	// Set NTSC options
	switch (conf.video_ntsc_mode) {
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
		
		default: break;
	}
	
	// Set xBR options
	if (conf.video_filter == 2) {
		video.SetCornerRounding(conf.video_xbr_corner_rounding);
		video.SetBlend(conf.video_xbr_pixel_blending);
	}
	
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

	if (NES_FAILED(video.SetRenderState(renderstate))) {
		fprintf(stderr, "Nestopia core rejected render state\n");
		exit(1);
	}
}

void video_set_dimensions() {
	// Set up the video dimensions
	int scalefactor = conf.video_scale_factor;
	
	switch(conf.video_filter) {
		case 0:	// None
			basesize.w = Video::Output::WIDTH;
			basesize.h = Video::Output::HEIGHT;
			conf.video_tv_aspect == true ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w * scalefactor;
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
			if (conf.video_filter == 5 && scalefactor == 4) {
				fprintf(stderr, "error: ScaleX filter cannot scale to 4x\n");
				conf.video_scale_factor = scalefactor = 3;
			}
			
			basesize.w = Video::Output::WIDTH * scalefactor;
			basesize.h = Video::Output::HEIGHT * scalefactor;
			conf.video_tv_aspect == true ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = basesize.w;
			rendersize.h = basesize.h;
			break;
		
		case 4: // 2xSaI
			basesize.w = Video::Output::WIDTH * 2;
			basesize.h = Video::Output::HEIGHT * 2;
			conf.video_tv_aspect == true ? rendersize.w = TV_WIDTH * scalefactor : rendersize.w = Video::Output::WIDTH * scalefactor;
			rendersize.h = Video::Output::HEIGHT * scalefactor;
			break;
	}

	if (!conf.video_unmask_overscan) {
		rendersize.h -= (OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor;
	}
	
	// Calculate the aspect from the height because it's smaller
	float aspect = (float)displaymode.h / (float)rendersize.h;
	
	if (!conf.video_stretch_aspect && conf.video_fullscreen && sdlwindow) {
		rendersize.h *= aspect;
		rendersize.w *= aspect;
	}
	else if (conf.video_fullscreen && sdlwindow) {
		rendersize.h = displaymode.h;
		rendersize.w = displaymode.w;
	}
	
	#ifdef _GTK
	if (!conf.misc_disable_gui) {
		SDL_SetWindowSize(embedwindow, rendersize.w, rendersize.h);
		gtkui_resize();
	}
	#endif
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void video_set_cursor() {
	// Set the cursor to what it needs to be
	int cursor;
	bool zapper;

	if (Input(emulator).GetConnectedController(0) == 5 ||
		Input(emulator).GetConnectedController(1) == 5) {
		zapper = true;
		cursor_set_crosshair();
	}
	else {
		zapper = false;
		cursor_set_default();
	}
	
	if (conf.video_fullscreen) { cursor = zapper; }
	else { cursor = true; }
		
	SDL_ShowCursor(cursor);
}

void video_set_title(const char *title) {
	// Set the window title
	SDL_SetWindowTitle(sdlwindow, title);
}

long video_lock_screen(void*& ptr) {
	ptr = videobuf;
	return basesize.w * 4;
}

void video_unlock_screen(void*) {
	opengl_blit();
}
