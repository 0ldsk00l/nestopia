#ifndef _VIDEO_H_
#define _VIDEO_H_

#define TV_WIDTH 292
#define OVERSCAN_LEFT 0
#define OVERSCAN_RIGHT 0
#define OVERSCAN_BOTTOM 8
#define OVERSCAN_TOP 8

typedef struct {
	int w;
	int h;
} dimensions;

void opengl_init_structures();
void opengl_cleanup();
void opengl_blit();

void video_init();
void video_create();
void video_toggle_fullscreen();
void video_toggle_filter();
void video_toggle_scalefactor();
void video_set_filter();
void video_set_params();

long video_lock_screen(void*& ptr);
void video_unlock_screen(void*);

#endif
