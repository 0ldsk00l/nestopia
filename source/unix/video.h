#ifndef _VIDEO_H_
#define _VIDEO_H_

#define TV_WIDTH 292
#define PAL_TV_WIDTH 320
#define OVERSCAN_LEFT 0
#define OVERSCAN_RIGHT 0
#define OVERSCAN_BOTTOM 8
#define OVERSCAN_TOP 8

#define VIDBUF_MAXSIZE 31457280

#include <epoxy/gl.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_endian.h>
#ifdef _APPLE
#include <OpenGL/gl.h>
#endif

typedef struct {
	int w;
	int h;
} dimensions_t;

void ogl_init();
void ogl_deinit();
void ogl_render();

void video_init();
void video_create_sdlwindow();
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
