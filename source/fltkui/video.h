#pragma once

#include "setmanager.h"
#include "jgmanager.h"

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

void nst_video_rehash();

void nst_video_resize(int w, int h);

void video_init(SettingManager *s, JGManager *j);

void video_scaled_coords(int x, int y, int *xcoord, int *ycoord);

void video_screenshot(const char* filename);
void video_clear_buffer();
void video_disp_nsf();
void nst_video_print(const char *text, int xpos, int ypos, int seconds, bool bg);
void nst_video_print_time(const char *timebuf, bool drawtime);
void nst_video_text_draw(const char *text, int xpos, int ypos, bool bg);
void nst_video_text_match(const char *text, int *xpos, int *ypos, int strpos);

void nst_video_dimensions(int *w, int *h);
