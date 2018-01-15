#ifndef _SDLVIDEO_H_
#define _SDLVIDEO_H_

void nstsdl_video_create();
void nstsdl_video_destroy();

dimensions_t nstsdl_video_get_dimensions();
void nstsdl_video_resize();

void nstsdl_video_set_cursor();
void nstsdl_video_set_title(const char *title);

void nstsdl_video_swapbuffers();

void nstsdl_video_toggle_fullscreen();
void nstsdl_video_toggle_filter();
void nstsdl_video_toggle_scale();

#endif
