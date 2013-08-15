#ifndef _CONFIG_H_
#define _CONFIG_H_
#include <glib.h>
typedef struct
{
	// Video
	gint video_renderer;
	gint video_filter;
	gint video_scale_factor;
	gint video_ntsc_mode;
	gint video_xbr_corner_rounding;
	gboolean video_xbr_pixel_blending;
	gboolean video_tv_aspect;
	gboolean video_mask_overscan;
	gboolean video_fullscreen;
	gboolean video_stretch_fullscreen;
	gboolean video_unlimited_sprites;
	
	// Audio
	gint audio_api;
	gint audio_sample_rate;
	gint audio_volume;
	gint audio_surround_multiplier;
	gboolean audio_surround;
	gboolean audio_stereo;
	gboolean audio_stereo_exciter;
	
	// Misc
	gint misc_video_region;
	gint misc_default_system;
	gboolean misc_soft_patching;
} settings;

void read_config_file();
void write_config_file();
#endif
