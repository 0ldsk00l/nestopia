/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2014 R. Danbrook
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
		fprintf(fp, "linear_filter=%d\n", conf.video_linear_filter);
		fprintf(fp, "tv_aspect=%d\n", conf.video_tv_aspect);
		fprintf(fp, "mask_overscan=%d\n", conf.video_mask_overscan);
		fprintf(fp, "fullscreen=%d\n", conf.video_fullscreen);
		fprintf(fp, "preserve_aspect=%d\n", conf.video_preserve_aspect);
		fprintf(fp, "unlimited_sprites=%d\n", conf.video_unlimited_sprites);
		fprintf(fp, "xbr_pixel_blending=%d\n", conf.video_xbr_pixel_blending);
		fprintf(fp, "\n"); // End of Section
		
		// Audio
		fprintf(fp, "[audio]\n");
		fprintf(fp, "api=%d\n", conf.audio_api);
		fprintf(fp, "stereo=%d\n", conf.audio_stereo);
		fprintf(fp, "sample_rate=%d\n", conf.audio_sample_rate);
		fprintf(fp, "volume=%d\n", conf.audio_volume);
		fprintf(fp, "vol_sq1=%d\n", conf.audio_vol_sq1);
		fprintf(fp, "vol_sq2=%d\n", conf.audio_vol_sq2);
		fprintf(fp, "vol_tri=%d\n", conf.audio_vol_tri);
		fprintf(fp, "vol_noise=%d\n", conf.audio_vol_noise);
		fprintf(fp, "vol_dpcm=%d\n", conf.audio_vol_dpcm);
		fprintf(fp, "vol_fds=%d\n", conf.audio_vol_fds);
		fprintf(fp, "vol_mmc5=%d\n", conf.audio_vol_mmc5);
		fprintf(fp, "vol_vrc6=%d\n", conf.audio_vol_vrc6);
		fprintf(fp, "vol_vrc7=%d\n", conf.audio_vol_vrc7);
		fprintf(fp, "vol_n163=%d\n", conf.audio_vol_n163);
		fprintf(fp, "vol_s5b=%d\n", conf.audio_vol_s5b);
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
	conf.video_linear_filter = false;
	conf.video_tv_aspect = false;
	conf.video_mask_overscan = true;
	conf.video_fullscreen = false;
	conf.video_preserve_aspect = false;
	conf.video_unlimited_sprites = true;
	conf.video_xbr_pixel_blending = true;
	
	// Audio
	conf.audio_api = 1;
	conf.audio_stereo = false;
	conf.audio_sample_rate = 48000;
	conf.audio_volume = 85;
	conf.audio_vol_sq1 = 85;
	conf.audio_vol_sq2 = 85;
	conf.audio_vol_tri = 85;
	conf.audio_vol_noise = 85;
	conf.audio_vol_dpcm = 85;
	conf.audio_vol_fds = 85;
	conf.audio_vol_mmc5 = 85;
	conf.audio_vol_vrc6 = 85;
	conf.audio_vol_vrc7 = 85;
	conf.audio_vol_n163 = 85;
	conf.audio_vol_s5b = 85;
	
	// Timing
	conf.timing_speed = 60;
	conf.timing_altspeed = 180;
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
	if (MATCH("video", "filter")) { pconfig->video_filter = atoi(value); }
	else if (MATCH("video", "scale_factor")) { pconfig->video_scale_factor = atoi(value); }
	else if (MATCH("video", "palette_mode")) { pconfig->video_palette_mode = atoi(value); }
	else if (MATCH("video", "decoder")) { pconfig->video_decoder = atoi(value); }
	else if (MATCH("video", "brightness")) { pconfig->video_brightness = atoi(value); }
	else if (MATCH("video", "saturation")) { pconfig->video_saturation = atoi(value); }
	else if (MATCH("video", "contrast")) { pconfig->video_contrast = atoi(value); }
	else if (MATCH("video", "hue")) { pconfig->video_hue = atoi(value); }
	else if (MATCH("video", "ntsc_mode")) { pconfig->video_ntsc_mode = atoi(value); }
	else if (MATCH("video", "xbr_corner_rounding")) { pconfig->video_xbr_corner_rounding = atoi(value); }
	else if (MATCH("video", "linear_filter")) { pconfig->video_linear_filter = atoi(value); }
	else if (MATCH("video", "tv_aspect")) { pconfig->video_tv_aspect = atoi(value); }
	else if (MATCH("video", "mask_overscan")) { pconfig->video_mask_overscan = atoi(value); }
	else if (MATCH("video", "fullscreen")) { pconfig->video_fullscreen = atoi(value); }
	else if (MATCH("video", "preserve_aspect")) { pconfig->video_preserve_aspect = atoi(value); }
	else if (MATCH("video", "unlimited_sprites")) { pconfig->video_unlimited_sprites = atoi(value); }
	else if (MATCH("video", "xbr_pixel_blending")) { pconfig->video_xbr_pixel_blending = atoi(value); }
	
	// Audio
	else if (MATCH("audio", "api")) { pconfig->audio_api = atoi(value); }
	else if (MATCH("audio", "stereo")) { pconfig->audio_stereo = atoi(value); }
	else if (MATCH("audio", "sample_rate")) { pconfig->audio_sample_rate = atoi(value); }
	else if (MATCH("audio", "volume")) { pconfig->audio_volume = atoi(value); }
	else if (MATCH("audio", "vol_sq1")) { pconfig->audio_vol_sq1 = atoi(value); }
	else if (MATCH("audio", "vol_sq2")) { pconfig->audio_vol_sq2 = atoi(value); }
	else if (MATCH("audio", "vol_tri")) { pconfig->audio_vol_tri = atoi(value); }
	else if (MATCH("audio", "vol_noise")) { pconfig->audio_vol_noise = atoi(value); }
	else if (MATCH("audio", "vol_dpcm")) { pconfig->audio_vol_dpcm = atoi(value); }
	else if (MATCH("audio", "vol_fds")) { pconfig->audio_vol_fds = atoi(value); }
	else if (MATCH("audio", "vol_mmc5")) { pconfig->audio_vol_mmc5 = atoi(value); }
	else if (MATCH("audio", "vol_vrc6")) { pconfig->audio_vol_vrc6 = atoi(value); }
	else if (MATCH("audio", "vol_vrc7")) { pconfig->audio_vol_vrc7 = atoi(value); }
	else if (MATCH("audio", "vol_n163")) { pconfig->audio_vol_n163 = atoi(value); }
	else if (MATCH("audio", "vol_s5b")) { pconfig->audio_vol_s5b = atoi(value); }
	
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
