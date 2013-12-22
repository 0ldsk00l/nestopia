/*
 * Nestopia UE
 * 
 * Copyright (C) 2013 R. Danbrook
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
#include <string.h>

#include "config.h"
#include "ini.h"

settings conf;

char confpath[256];
extern char nstdir[256];

void config_file_read() {
	// Read the config file

	snprintf(confpath, sizeof(confpath), "%snestopia.conf", nstdir);

	if (ini_parse(confpath, config_match, &conf) < 0) {
		fprintf(stderr, "Failed to read config file %s: Using defaults.\n", confpath);
		config_set_default();
	}
}

void config_file_write() {
	// Write the config file
	
	FILE *fp = fopen(confpath, "w");
	if (fp != NULL)	{
		// Video
		fprintf(fp, "[video]\n");
		fprintf(fp, "renderer=%d\n", conf.video_renderer);
		fprintf(fp, "filter=%d\n", conf.video_filter);
		fprintf(fp, "scale_factor=%d\n", conf.video_scale_factor);
		fprintf(fp, "palette_mode=%d\n", conf.video_palette_mode);
		fprintf(fp, "decoder=%d\n", conf.video_decoder);
		fprintf(fp, "brightness=%d\n", conf.video_brightness);
		fprintf(fp, "saturation=%d\n", conf.video_saturation);
		fprintf(fp, "contrast=%d\n", conf.video_contrast);
		fprintf(fp, "hue=%d\n", conf.video_hue);
		fprintf(fp, "ntsc_mode=%d\n", conf.video_ntsc_mode);
		fprintf(fp, "xbr_corner_rounding=%d\n", conf.video_xbr_corner_rounding);
		fprintf(fp, "xbr_pixel_blending=%d\n", conf.video_xbr_pixel_blending);
		fprintf(fp, "tv_aspect=%d\n", conf.video_tv_aspect);
		fprintf(fp, "mask_overscan=%d\n", conf.video_mask_overscan);
		fprintf(fp, "fullscreen=%d\n", conf.video_fullscreen);
		fprintf(fp, "preserve_aspect=%d\n", conf.video_preserve_aspect);
		fprintf(fp, "unlimited_sprites=%d\n", conf.video_unlimited_sprites);
		fprintf(fp, "\n"); // End of Section
		
		// Audio
		fprintf(fp, "[audio]\n");
		fprintf(fp, "api=%d\n", conf.audio_api);
		fprintf(fp, "sample_rate=%d\n", conf.audio_sample_rate);
		fprintf(fp, "volume=%d\n", conf.audio_volume);
		fprintf(fp, "surround_multiplier=%d\n", conf.audio_surround_multiplier);
		fprintf(fp, "surround=%d\n", conf.audio_surround);
		fprintf(fp, "stereo=%d\n", conf.audio_stereo);
		fprintf(fp, "stereo_exciter=%d\n", conf.audio_stereo_exciter);
		fprintf(fp, "\n"); // End of Section
		
		// Timing
		fprintf(fp, "[timing]\n");
		fprintf(fp, "speed=%d\n", conf.timing_speed);
		fprintf(fp, "altspeed=%d\n", conf.timing_altspeed);
		fprintf(fp, "vsync=%d\n", conf.timing_vsync);
		fprintf(fp, "\n"); // End of Section
		
		// Misc
		fprintf(fp, "[misc]\n");
		fprintf(fp, "video_region=%d\n", conf.misc_video_region);
		fprintf(fp, "default_system=%d\n", conf.misc_default_system);
		fprintf(fp, "soft_patching=%d\n", conf.misc_soft_patching);
		fprintf(fp, "suppress_screensaver=%d\n", conf.misc_suppress_screensaver);
		fprintf(fp, "disable_gui=%d\n", conf.misc_disable_gui);
		
		fclose(fp);
	}
	else {
		fprintf(stderr, "Failed to write config file %s.\n", confpath);
	}
}

void config_set_default() {
	
	// Video
	conf.video_renderer = 1;
	conf.video_filter = 0;
	conf.video_scale_factor = 2;
	conf.video_palette_mode = 0;
	conf.video_decoder = 0;
	conf.video_brightness = 0; // -100 to 100
	conf.video_saturation = 0; // -100 to 100
	conf.video_contrast = 0; // -100 to 100
	conf.video_hue = 0; // -45 to 45
	conf.video_ntsc_mode = 0;
	conf.video_xbr_corner_rounding = 0;
	
	conf.video_xbr_pixel_blending = true;
	conf.video_tv_aspect = false;
	conf.video_mask_overscan = true;
	conf.video_fullscreen = false;
	conf.video_preserve_aspect = false;
	conf.video_unlimited_sprites = true;
	
	// Audio
	conf.audio_api = 0;
	conf.audio_sample_rate = 48000;
	conf.audio_volume = 85;
	conf.audio_surround_multiplier = 50;
	
	conf.audio_surround = false;
	conf.audio_stereo = false;
	conf.audio_stereo_exciter = false;
	
	// Timing
	conf.timing_speed = 60;
	conf.timing_altspeed = 240;
	conf.timing_vsync = true;
	
	// Misc
	conf.misc_video_region = 0;
	conf.misc_default_system = 0;
	conf.misc_soft_patching = true;
	conf.misc_suppress_screensaver = true;
	conf.misc_disable_gui = true;
}

static int config_match(void* user, const char* section, const char* name, const char* value) {
	// Match values from config file and populate live config
	settings* pconfig = (settings*)user;
	
	// Video
	if (MATCH("video", "renderer")) { pconfig->video_renderer = atoi(value); }
	else if (MATCH("video", "filter")) { pconfig->video_filter = atoi(value); }
	else if (MATCH("video", "scale_factor")) { pconfig->video_scale_factor = atoi(value); }
	else if (MATCH("video", "palette_mode")) { pconfig->video_palette_mode = atoi(value); }
	else if (MATCH("video", "decoder")) { pconfig->video_decoder = atoi(value); }
	else if (MATCH("video", "brightness")) { pconfig->video_brightness = atoi(value); }
	else if (MATCH("video", "saturation")) { pconfig->video_saturation = atoi(value); }
	else if (MATCH("video", "contrast")) { pconfig->video_contrast = atoi(value); }
	else if (MATCH("video", "hue")) { pconfig->video_hue = atoi(value); }
	else if (MATCH("video", "ntsc_mode")) { pconfig->video_ntsc_mode = atoi(value); }
	else if (MATCH("video", "xbr_corner_rounding")) { pconfig->video_xbr_corner_rounding = atoi(value); }
	else if (MATCH("video", "xbr_pixel_blending")) { pconfig->video_xbr_pixel_blending = atoi(value); }
	else if (MATCH("video", "tv_aspect")) { pconfig->video_tv_aspect = atoi(value); }
	else if (MATCH("video", "mask_overscan")) { pconfig->video_mask_overscan = atoi(value); }
	else if (MATCH("video", "fullscreen")) { pconfig->video_fullscreen = atoi(value); }
	else if (MATCH("video", "preserve_aspect")) { pconfig->video_preserve_aspect = atoi(value); }
	else if (MATCH("video", "unlimited_sprites")) { pconfig->video_unlimited_sprites = atoi(value); }
	
	// Audio
	else if (MATCH("audio", "api")) { pconfig->audio_api = atoi(value); }
	else if (MATCH("audio", "sample_rate")) { pconfig->audio_sample_rate = atoi(value); }
	else if (MATCH("audio", "volume")) { pconfig->audio_volume = atoi(value); }
	else if (MATCH("audio", "surround")) { pconfig->audio_surround = atoi(value); }
	else if (MATCH("audio", "surround_multiplier")) { pconfig->audio_surround_multiplier = atoi(value); }
	else if (MATCH("audio", "stereo")) { pconfig->audio_stereo = atoi(value); }
	else if (MATCH("audio", "stereo_exciter")) { pconfig->audio_stereo_exciter = atoi(value); }
	
	// Timing
	else if (MATCH("timing", "speed")) { pconfig->timing_speed = atoi(value); }
	else if (MATCH("timing", "altspeed")) { pconfig->timing_altspeed = atoi(value); }
	else if (MATCH("timing", "vsync")) { pconfig->timing_vsync = atoi(value); }
    
    // Misc
    else if (MATCH("misc", "video_region")) { pconfig->misc_video_region = atoi(value); }
    else if (MATCH("misc", "default_system")) { pconfig->misc_default_system = atoi(value); }
    else if (MATCH("misc", "soft_patching")) { pconfig->misc_soft_patching = atoi(value); }
    else if (MATCH("misc", "suppress_screensaver")) { pconfig->misc_suppress_screensaver = atoi(value); }
    else if (MATCH("misc", "disable_gui")) { pconfig->misc_disable_gui = atoi(value); }
    
    else { return 0; }
    return 1;
}
