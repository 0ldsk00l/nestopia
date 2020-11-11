/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
 * Copyright (C) 2012-2020 R. Danbrook
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
#include <time.h>

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiInput.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiNsf.hpp"

#include "nstcommon.h"
#include "video.h"
#include "config.h"
#include "font.h"
#include "png.h"

using namespace Nes::Api;

static int overscan_offset, overscan_height;

static uint32_t videobuf[VIDBUF_MAXSIZE]; // Maximum possible internal size

static Video::RenderState::Filter filter;
static Video::RenderState renderstate;

static dimensions_t basesize, rendersize, screensize;
static osdtext_t osdtext;

extern void *custompalette;

extern nstpaths_t nstpaths;
extern Emulator emulator;

// Shader sources
const GLchar* vshader_src =
	"#version 150 core\n"
	"in vec2 position;"
	"in vec2 texcoord;"
	"out vec2 outcoord;"
	"void main() {"
	"	outcoord = texcoord;"
	"	gl_Position = vec4(position, 0.0, 1.0);"
	"}";

const GLchar* fshader_src =
	"#version 150 core\n"
	"in vec2 outcoord;"
	"out vec4 fragcolor;"
	"uniform sampler2D nestex;"
	"void main() {"
	"	fragcolor = texture(nestex, outcoord);"
	"}";

GLuint vao;
GLuint vbo;
GLuint vshader;
GLuint fshader;
GLuint gl_shader_prog = 0;
GLuint gl_texture_id = 0;

void nst_ogl_init() {
	// Initialize OpenGL
	
	float vertices[] = {
		-1.0f, -1.0f,	// Vertex 1 (X, Y)
		-1.0f, 1.0f,	// Vertex 2 (X, Y)
		1.0f, -1.0f,	// Vertex 3 (X, Y)
		1.0f, 1.0f,		// Vertex 4 (X, Y)
		0.0, 1.0,		// Texture 1 (X, Y)
		0.0, 0.0,		// Texture 2 (X, Y)
		1.0, 1.0,		// Texture 3 (X, Y)
		1.0, 0.0		// Texture 4 (X, Y)
	};
	
	GLint status;
	
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	
	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	vshader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vshader, 1, &vshader_src, NULL);
	glCompileShader(vshader);
	
	glGetShaderiv(vshader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) { fprintf(stderr, "Failed to compile vertex shader\n"); }
	
	fshader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fshader, 1, &fshader_src, NULL);
	glCompileShader(fshader);
	
	glGetShaderiv(fshader, GL_COMPILE_STATUS, &status);
	if (status == GL_FALSE) { fprintf(stderr, "Failed to compile fragment shader\n"); }
	
	GLuint gl_shader_prog = glCreateProgram();
	glAttachShader(gl_shader_prog, vshader);
	glAttachShader(gl_shader_prog, fshader);
	
	glLinkProgram(gl_shader_prog);
	
	glValidateProgram(gl_shader_prog);
	glGetProgramiv(gl_shader_prog, GL_LINK_STATUS, &status);
	if (status == GL_FALSE) { fprintf(stderr, "Failed to link shader program\n"); }
	
	glUseProgram(gl_shader_prog);
	
	GLint posAttrib = glGetAttribLocation(gl_shader_prog, "position");
	glEnableVertexAttribArray(posAttrib);
	glVertexAttribPointer(posAttrib, 2, GL_FLOAT, GL_FALSE, 0, 0);
	
	GLint texAttrib = glGetAttribLocation(gl_shader_prog, "texcoord");
	glEnableVertexAttribArray(texAttrib);
	glVertexAttribPointer(texAttrib, 2, GL_FLOAT, GL_FALSE, 0, (void*)(8 * sizeof(GLfloat)));
	
	glGenTextures(1, &gl_texture_id);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, gl_texture_id);
	
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, conf.video_linear_filter ? GL_LINEAR : GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	
	conf.video_fullscreen ? 
	glViewport(screensize.w / 2.0f - rendersize.w / 2.0f, 0, rendersize.w, rendersize.h) :
	glViewport(0, 0, rendersize.w, rendersize.h);
	
	glUniform1i(glGetUniformLocation(gl_shader_prog, "nestex"), 0);
}

void nst_ogl_deinit() {
	// Deinitialize OpenGL
	if (gl_texture_id) { glDeleteTextures(1, &gl_texture_id); }
	if (gl_shader_prog) { glDeleteProgram(gl_shader_prog); }
	if (vshader) { glDeleteShader(vshader); }
	if (fshader) { glDeleteShader(fshader); }
	if (vao) { glDeleteVertexArrays(1, &vao); }
	if (vbo) { glDeleteBuffers(1, &vbo); }
}

#ifdef _WITH_NOTCURSES
int nst_notcurses_render(struct notcurses *nc) {
	struct ncvisual_options vopts = {
		.n = notcurses_stdplane(nc),
		.leny = overscan_height,
		.lenx = basesize.w,
		.blitter = NCBLIT_2x1,
	};
	if (ncblit_bgrx(videobuf + overscan_offset, 4 * basesize.w, &vopts) < 0) {
		return -1;
	}
	if (notcurses_render(nc)) {
		return -1;
	}
	return 0;
}
#endif

void nst_ogl_render() {
	// Render the scene

	glTexImage2D(GL_TEXTURE_2D,
				0,
				GL_RGBA,
				basesize.w,
				overscan_height,
				0,
				GL_BGRA,
				GL_UNSIGNED_BYTE,
		videobuf + overscan_offset);
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void nst_video_refresh() {
	// Refresh the video settings
	
	nst_ogl_deinit();
	
	nst_ogl_init();
}

void video_init() {
	// Initialize video

	nst_ogl_deinit();
	
	video_set_dimensions();
	video_set_filter();
	
	nst_ogl_init();
	
	if (nst_nsf()) { video_clear_buffer(); video_disp_nsf(); }
}

void video_toggle_fullscreen() {
	// Toggle between fullscreen and window mode
	if (!nst_playing()) { return; }
	conf.video_fullscreen ^= 1;
}

void video_toggle_filter() {
	conf.video_filter++;
	if (conf.video_filter > 5) { conf.video_filter = 0; }
	//video_init();
	//nst_video_refresh();
}

void video_toggle_filterupdate() {
	// Clear the filter update flag
	Video video(emulator);
	video.ClearFilterUpdateFlag();
}

void video_toggle_scalefactor() {
	// Toggle video scale factor
	conf.video_scale_factor++;
	if (conf.video_scale_factor > 8) { conf.video_scale_factor = 1; }
	//video_init();
}

void video_set_filter() {
	// Set the filter
	Video video(emulator);
	int scalefactor = conf.video_scale_factor;
	if (conf.video_scale_factor > 4) { scalefactor = 4; }
	if ((conf.video_scale_factor > 3) && (conf.video_filter == 5)) { scalefactor = 3; }
	
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
			break;
		
		case 2: // Custom
			video.GetPalette().SetMode(Video::Palette::MODE_CUSTOM);
			video.GetPalette().SetCustom((const unsigned char (*)[3])custompalette, Video::Palette::EXT_PALETTE);
			break;
		
		default: break;
	}
	
	// Set YUV Decoder/Picture options
	if (video.GetPalette().GetMode() == Video::Palette::MODE_YUV) {
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
	}
	
	video.SetBrightness(conf.video_brightness);
	video.SetSaturation(conf.video_saturation);
	video.SetContrast(conf.video_contrast);
	video.SetHue(conf.video_hue);
	
	// Set NTSC options
	if (conf.video_filter == 1) {
		switch (conf.video_ntsc_mode) {
			case 0:	// Composite
				video.SetSaturation(Video::DEFAULT_SATURATION_COMP);
				video.SetSharpness(Video::DEFAULT_SHARPNESS_COMP);
				video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_COMP);
				video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_COMP);
				video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_COMP);
				video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_COMP);
				break;
			
			case 1:	// S-Video
				video.SetSaturation(Video::DEFAULT_SATURATION_SVIDEO);
				video.SetSharpness(Video::DEFAULT_SHARPNESS_SVIDEO);
				video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_SVIDEO);
				video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_SVIDEO);
				video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO);
				video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_SVIDEO);
				break;
			
			case 2:	// RGB
				video.SetSaturation(Video::DEFAULT_SATURATION_RGB);
				video.SetSharpness(Video::DEFAULT_SHARPNESS_RGB);
				video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_RGB);
				video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_RGB);
				video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_RGB);
				video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_RGB);
				break;
			
			case 3: // Monochrome
				video.SetSaturation(Video::DEFAULT_SATURATION_MONO);
				video.SetSharpness(Video::DEFAULT_SHARPNESS_MONO);
				video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_MONO);
				video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_MONO);
				video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_MONO);
				video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_MONO);
				break;
			
			case 4: // Custom
				video.SetSaturation(conf.video_saturation);
				video.SetSharpness(conf.video_ntsc_sharpness);
				video.SetColorResolution(conf.video_ntsc_resolution);
				video.SetColorBleed(conf.video_ntsc_bleed);
				video.SetColorArtifacts(conf.video_ntsc_artifacts);
				video.SetColorFringing(conf.video_ntsc_fringing);
				break;
			
			default: break;
		}
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
	
	int e = 1; // Check Endianness
	if ((int)*((unsigned char *)&e) == 1) { // Little Endian
		renderstate.bits.mask.r = 0x00ff0000;
		renderstate.bits.mask.g = 0x0000ff00;
		renderstate.bits.mask.b = 0x000000ff;
	}
	else { // Big Endian
		renderstate.bits.mask.r = 0x000000ff;
		renderstate.bits.mask.g = 0xff000000;
		renderstate.bits.mask.b = 0x00ff0000;
	}
	
	if (NES_FAILED(video.SetRenderState(renderstate))) {
		fprintf(stderr, "Nestopia core rejected render state\n");
		exit(1);
	}
}

dimensions_t nst_video_get_dimensions_render() {
	// Return the dimensions of the rendered video
	return rendersize;
}

dimensions_t nst_video_get_dimensions_screen() {
	// Return the dimensions of the screen
	return screensize;
}

void nst_video_set_dimensions_screen(dimensions_t scrsize) {
	screensize = scrsize;
}

void video_set_dimensions() {
	// Set up the video dimensions
	int scalefactor = conf.video_scale_factor;
	if (conf.video_scale_factor > 4) { scalefactor = 4; }
	if ((conf.video_scale_factor > 3) && (conf.video_filter == 5)) { scalefactor = 3; }
	int wscalefactor = conf.video_scale_factor;
	int tvwidth = nst_pal() ? PAL_TV_WIDTH : TV_WIDTH;
	
	switch(conf.video_filter) {
		case 0:	// None
			basesize.w = Video::Output::WIDTH;
			basesize.h = Video::Output::HEIGHT;
			conf.video_tv_aspect == true ? rendersize.w = tvwidth * wscalefactor : rendersize.w = basesize.w * wscalefactor;
			rendersize.h = basesize.h * wscalefactor;
			overscan_offset = basesize.w * OVERSCAN_TOP;
			overscan_height = basesize.h - OVERSCAN_TOP - OVERSCAN_BOTTOM;
			break;

		case 1: // NTSC
			basesize.w = Video::Output::NTSC_WIDTH;
			rendersize.w = (basesize.w / 2) * wscalefactor;
			basesize.h = Video::Output::HEIGHT;
			rendersize.h = basesize.h * wscalefactor;
			overscan_offset = basesize.w * OVERSCAN_TOP;
			overscan_height = basesize.h - OVERSCAN_TOP - OVERSCAN_BOTTOM;
			break;

		case 2: // xBR
		case 3: // HqX
		case 5: // ScaleX
			basesize.w = Video::Output::WIDTH * scalefactor;
			basesize.h = Video::Output::HEIGHT * scalefactor;
			conf.video_tv_aspect == true ? rendersize.w = tvwidth * wscalefactor : rendersize.w = Video::Output::WIDTH * wscalefactor;;
			rendersize.h = Video::Output::HEIGHT * wscalefactor;
			overscan_offset = basesize.w * OVERSCAN_TOP * scalefactor;
			overscan_height = basesize.h - (OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor;
			break;
		
		case 4: // 2xSaI
			basesize.w = Video::Output::WIDTH * 2;
			basesize.h = Video::Output::HEIGHT * 2;
			conf.video_tv_aspect == true ? rendersize.w = tvwidth * wscalefactor : rendersize.w = Video::Output::WIDTH * wscalefactor;
			rendersize.h = Video::Output::HEIGHT * wscalefactor;
			overscan_offset = basesize.w * OVERSCAN_TOP * 2;
			overscan_height = basesize.h - (OVERSCAN_TOP + OVERSCAN_BOTTOM) * 2;
			break;
	}

	if (!conf.video_unmask_overscan) {
		rendersize.h -= (OVERSCAN_TOP + OVERSCAN_BOTTOM) * scalefactor;
	}
	else { overscan_offset = 0; overscan_height = basesize.h; }
	
	// Calculate the aspect from the height because it's smaller
	float aspect = (float)screensize.h / (float)rendersize.h;
	
	if (!conf.video_stretch_aspect && conf.video_fullscreen) {
		rendersize.h *= aspect;
		rendersize.w *= aspect;
	}
	else if (conf.video_fullscreen) {
		rendersize.h = screensize.h;
		rendersize.w = screensize.w;
	}
}

long video_lock_screen(void*& ptr) {
	ptr = videobuf;
	return basesize.w * 4;
}

void video_unlock_screen(void*) {
	
	int xscale = renderstate.width / Video::Output::WIDTH;;
	int yscale = renderstate.height / Video::Output::HEIGHT;
	
	if (osdtext.drawtext) {
		nst_video_text_draw(osdtext.textbuf, osdtext.xpos * xscale, osdtext.ypos * yscale, osdtext.bg);
		osdtext.drawtext--;
	}
	
	if (osdtext.drawtime) {
		nst_video_text_draw(osdtext.timebuf, 208 * xscale, 218 * yscale, false);
	}
}

void video_screenshot_flip(unsigned char *pixels, int width, int height, int bytes) {
	// Flip the pixels
	int rowsize = width * bytes;
	unsigned char *row = (unsigned char*)malloc(rowsize);
	unsigned char *low = pixels;
	unsigned char *high = &pixels[(height - 1) * rowsize];
	
	for (; low < high; low += rowsize, high -= rowsize) {
		memcpy(row, low, rowsize);
		memcpy(low, high, rowsize);
		memcpy(high, row, rowsize);
	}
	free(row);
}

void video_screenshot(const char* filename) {
	// Take a screenshot in .png format
	unsigned char *pixels;
	pixels = (unsigned char*)malloc(sizeof(unsigned char) * rendersize.w * rendersize.h * 4);
	
	// Read the pixels and flip them vertically
	glReadPixels(0, 0, rendersize.w, rendersize.h, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
	video_screenshot_flip(pixels, rendersize.w, rendersize.h, 4);
	
	if (filename == NULL) {
		// Set the filename
		char sshotpath[512];
		snprintf(sshotpath, sizeof(sshotpath), "%sscreenshots/%s-%ld-%d.png", nstpaths.nstdir, nstpaths.gamename, time(NULL), rand() % 899 + 100);
		
		// Save the file
		lodepng_encode32_file(sshotpath, (const unsigned char*)pixels, rendersize.w, rendersize.h);
		fprintf(stderr, "Screenshot: %s\n", sshotpath);
	}
	else {
		lodepng_encode32_file(filename, (const unsigned char*)pixels, rendersize.w, rendersize.h);
	}
	
	free(pixels);
}

void video_clear_buffer() {
	// Write black to the video buffer
	memset(videobuf, 0x00000000, VIDBUF_MAXSIZE);
}

void video_disp_nsf() {
	// Display NSF text
	Nsf nsf(emulator);
	
	int xscale = renderstate.width / Video::Output::WIDTH;;
	int yscale = renderstate.height / Video::Output::HEIGHT;;
	
	nst_video_text_draw(nsf.GetName(), 4 * xscale, 16 * yscale, false);
	nst_video_text_draw(nsf.GetArtist(), 4 * xscale, 28 * yscale, false);
	nst_video_text_draw(nsf.GetCopyright(), 4 * xscale, 40 * yscale, false);
	
	char currentsong[10];
	snprintf(currentsong, sizeof(currentsong), "%d / %d", nsf.GetCurrentSong() +1, nsf.GetNumSongs());
	nst_video_text_draw(currentsong, 4 * xscale, 52 * yscale, false);
	
	nst_ogl_render();
}

void nst_video_disp_inputconf(int type, int pnum, int bnum) {
	
	int xscale = renderstate.width / Video::Output::WIDTH;;
	int yscale = renderstate.height / Video::Output::HEIGHT;;
	
	char textbuf[32];
	char buttontext[8];
	
	if (type == 0) { snprintf(textbuf, sizeof(textbuf), "Player %d Keyboard Configuration", pnum + 1); }
	else { snprintf(textbuf, sizeof(textbuf), "Player %d Joystick Configuration", pnum + 1); }
	
	switch (bnum) {
		case 0: snprintf(buttontext, sizeof(buttontext), "Up"); break;
		case 1: snprintf(buttontext, sizeof(buttontext), "Down"); break;
		case 2: snprintf(buttontext, sizeof(buttontext), "Left"); break;
		case 3: snprintf(buttontext, sizeof(buttontext), "Right"); break;
		case 4: snprintf(buttontext, sizeof(buttontext), "Select"); break;
		case 5: snprintf(buttontext, sizeof(buttontext), "Start"); break;
		case 6: snprintf(buttontext, sizeof(buttontext), "A"); break;
		case 7: snprintf(buttontext, sizeof(buttontext), "B"); break;
		case 8: snprintf(buttontext, sizeof(buttontext), "Turbo A"); break;
		case 9: snprintf(buttontext, sizeof(buttontext), "Turbo B"); break;
	}
	
	video_clear_buffer();
	
	nst_video_text_draw(textbuf, 4 * xscale, 64 * yscale, false);
	nst_video_text_draw(buttontext, 112 * xscale, 128 * yscale, false);
	
	nst_ogl_render();
}

void nst_video_print(const char *text, int xpos, int ypos, int seconds, bool bg) {
	snprintf(osdtext.textbuf, sizeof(osdtext.textbuf), "%s", text);
	osdtext.xpos = xpos;
	osdtext.ypos = ypos;
	osdtext.drawtext = seconds * nst_pal() ? 50 : 60;
	osdtext.bg = bg;
}

void nst_video_print_time(const char *timebuf, bool drawtime) {
	snprintf(osdtext.timebuf, sizeof(osdtext.timebuf), "%s", timebuf);
	osdtext.drawtime = drawtime;
}

void nst_video_text_draw(const char *text, int xpos, int ypos, bool bg) {
	// Draw text on screen
	uint32_t w = 0xc0c0c0c0; // "White", actually Grey
	uint32_t b = 0x00000000; // Black
	uint32_t g = 0x00358570; // Nestopia UE Green
	uint32_t d = 0x00255f65; // Nestopia UE Dark Green
	
	int numchars = strlen(text);
	
	int letterypos;
	int letterxpos;
	int letternum = 0;
	
	if (bg) { // Draw background borders
		for (int i = 0; i < numchars * 8; i++) { // Rows above and below
			videobuf[(xpos + i) + ((ypos - 1) * renderstate.width)] = g;
			videobuf[(xpos + i) + ((ypos + 8) * renderstate.width)] = g;
		}
		for (int i = 0; i < 8; i++) { // Columns on both sides
			videobuf[(xpos - 1) + ((ypos + i) * renderstate.width)] = g;
			videobuf[(xpos + (numchars * 8)) + ((ypos + i) * renderstate.width)] = g;
		}
	}
	
	for (int tpos = 0; tpos < (8 * numchars); tpos+=8) {
		nst_video_text_match(text, &letterxpos, &letterypos, letternum);
		for (int row = 0; row < 8; row++) { // Draw Rows
			for (int col = 0; col < 8; col++) { // Draw Columns
				switch (nesfont[row + letterypos][col + letterxpos]) {
					case '.':
						videobuf[xpos + ((ypos + row) * renderstate.width) + (col + tpos)] = w;
						break;
					
					case '+':
						videobuf[xpos + ((ypos + row) * renderstate.width) + (col + tpos)] = g;
						break;
					
					default:
						if (bg) { videobuf[xpos + ((ypos + row) * renderstate.width) + (col + tpos)] = d; }
						break;
				}
			}
		}
		letternum++;
	}
}

void nst_video_text_match(const char *text, int *xpos, int *ypos, int strpos) {
	// Match letters to draw on screen
	switch (text[strpos]) {
		case ' ': *xpos = 0; *ypos = 0; break;
		case '!': *xpos = 8; *ypos = 0; break;
		case '"': *xpos = 16; *ypos = 0; break;
		case '#': *xpos = 24; *ypos = 0; break;
		case '$': *xpos = 32; *ypos = 0; break;
		case '%': *xpos = 40; *ypos = 0; break;
		case '&': *xpos = 48; *ypos = 0; break;
		case '\'': *xpos = 56; *ypos = 0; break;
		case '(': *xpos = 64; *ypos = 0; break;
		case ')': *xpos = 72; *ypos = 0; break;
		case '*': *xpos = 80; *ypos = 0; break;
		case '+': *xpos = 88; *ypos = 0; break;
		case ',': *xpos = 96; *ypos = 0; break;
		case '-': *xpos = 104; *ypos = 0; break;
		case '.': *xpos = 112; *ypos = 0; break;
		case '/': *xpos = 120; *ypos = 0; break;
		case '0': *xpos = 0; *ypos = 8; break;
		case '1': *xpos = 8; *ypos = 8; break;
		case '2': *xpos = 16; *ypos = 8; break;
		case '3': *xpos = 24; *ypos = 8; break;
		case '4': *xpos = 32; *ypos = 8; break;
		case '5': *xpos = 40; *ypos = 8; break;
		case '6': *xpos = 48; *ypos = 8; break;
		case '7': *xpos = 56; *ypos = 8; break;
		case '8': *xpos = 64; *ypos = 8; break;
		case '9': *xpos = 72; *ypos = 8; break;
		case ':': *xpos = 80; *ypos = 8; break;
		case ';': *xpos = 88; *ypos = 8; break;
		case '<': *xpos = 96; *ypos = 8; break;
		case '=': *xpos = 104; *ypos = 8; break;
		case '>': *xpos = 112; *ypos = 8; break;
		case '?': *xpos = 120; *ypos = 8; break;
		case '@': *xpos = 0; *ypos = 16; break;
		case 'A': *xpos = 8; *ypos = 16; break;
		case 'B': *xpos = 16; *ypos = 16; break;
		case 'C': *xpos = 24; *ypos = 16; break;
		case 'D': *xpos = 32; *ypos = 16; break;
		case 'E': *xpos = 40; *ypos = 16; break;
		case 'F': *xpos = 48; *ypos = 16; break;
		case 'G': *xpos = 56; *ypos = 16; break;
		case 'H': *xpos = 64; *ypos = 16; break;
		case 'I': *xpos = 72; *ypos = 16; break;
		case 'J': *xpos = 80; *ypos = 16; break;
		case 'K': *xpos = 88; *ypos = 16; break;
		case 'L': *xpos = 96; *ypos = 16; break;
		case 'M': *xpos = 104; *ypos = 16; break;
		case 'N': *xpos = 112; *ypos = 16; break;
		case 'O': *xpos = 120; *ypos = 16; break;
		case 'P': *xpos = 0; *ypos = 24; break;
		case 'Q': *xpos = 8; *ypos = 24; break;
		case 'R': *xpos = 16; *ypos = 24; break;
		case 'S': *xpos = 24; *ypos = 24; break;
		case 'T': *xpos = 32; *ypos = 24; break;
		case 'U': *xpos = 40; *ypos = 24; break;
		case 'V': *xpos = 48; *ypos = 24; break;
		case 'W': *xpos = 56; *ypos = 24; break;
		case 'X': *xpos = 64; *ypos = 24; break;
		case 'Y': *xpos = 72; *ypos = 24; break;
		case 'Z': *xpos = 80; *ypos = 24; break;
		case '[': *xpos = 88; *ypos = 24; break;
		case '\\': *xpos = 96; *ypos = 24; break;
		case ']': *xpos = 104; *ypos = 24; break;
		case '^': *xpos = 112; *ypos = 24; break;
		case '_': *xpos = 120; *ypos = 24; break;
		case '`': *xpos = 0; *ypos = 32; break;
		case 'a': *xpos = 8; *ypos = 32; break;
		case 'b': *xpos = 16; *ypos = 32; break;
		case 'c': *xpos = 24; *ypos = 32; break;
		case 'd': *xpos = 32; *ypos = 32; break;
		case 'e': *xpos = 40; *ypos = 32; break;
		case 'f': *xpos = 48; *ypos = 32; break;
		case 'g': *xpos = 56; *ypos = 32; break;
		case 'h': *xpos = 64; *ypos = 32; break;
		case 'i': *xpos = 72; *ypos = 32; break;
		case 'j': *xpos = 80; *ypos = 32; break;
		case 'k': *xpos = 88; *ypos = 32; break;
		case 'l': *xpos = 96; *ypos = 32; break;
		case 'm': *xpos = 104; *ypos = 32; break;
		case 'n': *xpos = 112; *ypos = 32; break;
		case 'o': *xpos = 120; *ypos = 32; break;
		case 'p': *xpos = 0; *ypos = 40; break;
		case 'q': *xpos = 8; *ypos = 40; break;
		case 'r': *xpos = 16; *ypos = 40; break;
		case 's': *xpos = 24; *ypos = 40; break;
		case 't': *xpos = 32; *ypos = 40; break;
		case 'u': *xpos = 40; *ypos = 40; break;
		case 'v': *xpos = 48; *ypos = 40; break;
		case 'w': *xpos = 56; *ypos = 40; break;
		case 'x': *xpos = 64; *ypos = 40; break;
		case 'y': *xpos = 72; *ypos = 40; break;
		case 'z': *xpos = 80; *ypos = 40; break;
		case '{': *xpos = 88; *ypos = 40; break;
		case '|': *xpos = 96; *ypos = 40; break;
		case '}': *xpos = 104; *ypos = 40; break;
		case '~': *xpos = 112; *ypos = 40; break;
		//case ' ': *xpos = 120; *ypos = 40; break; // Triangle
		default: *xpos = 0; *ypos = 0; break;
	}
}
