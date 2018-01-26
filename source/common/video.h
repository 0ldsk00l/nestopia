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
#ifdef _APPLE
#include <OpenGL/gl.h>
#endif

typedef struct {
	int w;
	int h;
} dimensions_t;

typedef struct {
	int xpos;
	int ypos;
	char textbuf[32];
	char timebuf[6];
	int drawtext;
	bool drawtime;
	bool bg;
} osdtext_t;

void nst_ogl_init();
void nst_ogl_deinit();
void nst_ogl_render();

void video_init();
void video_toggle_fullscreen();
void video_toggle_filter();
void video_toggle_filterupdate();
void video_toggle_scalefactor();
void video_set_filter();

dimensions_t nst_video_get_dimensions_render();
dimensions_t nst_video_get_dimensions_screen();
void nst_video_set_dimensions_screen(dimensions_t scrsize);
void video_set_dimensions();

long video_lock_screen(void*& ptr);
void video_unlock_screen(void*);
void video_screenshot(const char* filename);
void video_clear_buffer();
void video_disp_nsf();
void nst_video_disp_inputconf(int type, int pnum, int bnum);
void nst_video_print(const char *text, int xpos, int ypos, int seconds, bool bg);
void nst_video_print_time(const char *timebuf, bool drawtime);
void nst_video_text_draw(const char *text, int xpos, int ypos, bool bg);
void nst_video_text_match(const char *text, int *xpos, int *ypos, int strpos);

#endif
