#ifndef _VIDEO_H_
#define _VIDEO_H_

#define TV_WIDTH 292
#define OVERSCAN_LEFT 0
#define OVERSCAN_RIGHT 0
#define OVERSCAN_BOTTOM 8
#define OVERSCAN_TOP 8

#include <epoxy/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_endian.h>
#ifdef _APPLE
#include <OpenGL/gl.h>
#endif

// This is part of an elaborate hack to embed the SDL window
struct SDL_Window {
	const void *magic;
	Uint32 id;
	char *title;
	SDL_Surface *icon;
	int x, y;
	int w, h;
	int min_w, min_h;
	int max_w, max_h;
	Uint32 flags;
};
typedef struct SDL_Window SDL_Window;
//

typedef struct {
	int w;
	int h;
} dimensions_t;

void ogl_init();
void ogl_deinit();
void ogl_render();

void video_init();
void video_create_standalone();
void video_create_embedded();
void video_create();
void video_swapbuffers();
void video_destroy();
void video_toggle_fullscreen();
void video_toggle_filter();
void video_toggle_filterupdate();
void video_toggle_scalefactor();
void video_set_filter();
void video_set_dimensions();
void video_set_cursor();
void video_set_title(const char *title);

long video_lock_screen(void*& ptr);
void video_unlock_screen(void*);
void video_screenshot(const char* filename);
void video_clear_buffer();
void video_disp_nsf();
void video_text_draw(const char *text, int xpos, int ypos);
void video_text_match(const char *text, int *xpos, int *ypos, int strpos);

#endif
