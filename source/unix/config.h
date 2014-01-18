#ifndef _CONFIG_H_
#define _CONFIG_H_

typedef struct {
	
	// Video
	int video_filter;
	int video_scale_factor;
	int video_palette_mode;
	int video_decoder;
	int video_brightness;
	int video_saturation;
	int video_contrast;
	int video_hue;
	int video_ntsc_mode;
	int video_xbr_corner_rounding;
	bool video_linear_filter;
	bool video_tv_aspect;
	bool video_mask_overscan;
	bool video_fullscreen;
	bool video_preserve_aspect;
	bool video_unlimited_sprites;
	bool video_xbr_pixel_blending;
	
	// Audio
	int audio_api;
	int audio_sample_rate;
	int audio_volume;
	bool audio_stereo;
	
	// Timing
	int timing_speed;
	int timing_altspeed;
	bool timing_vsync;
	
	// Misc
	int misc_video_region;
	int misc_default_system;
	bool misc_soft_patching;
	bool misc_suppress_screensaver;
	bool misc_disable_gui;
} settings;

void config_file_read();
void config_file_write();
void config_set_default();
static int config_match(void* user, const char* section, const char* name, const char* value);
#endif
