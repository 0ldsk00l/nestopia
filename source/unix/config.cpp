/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2016 R. Danbrook
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

#include "main.h"
#include "config.h"
#include "ini.h"

settings_t conf;

char confpath[256];
extern nstpaths_t nstpaths;

void config_file_read() {
	// Read the config file

	snprintf(confpath, sizeof(confpath), "%snestopia.conf", nstpaths.nstdir);

	if (ini_parse(confpath, config_match, &conf) < 0) {
		fprintf(stderr, "Failed to read config file %s: Using defaults.\n", confpath);
	}
}

void config_file_write() {
	// Write the config file
	
	FILE *fp = fopen(confpath, "w");
	if (fp != NULL)	{
		// Video
		fprintf(fp, "; Nestopia UE Configuration File\n\n");
		fprintf(fp, "[video]\n");
		fprintf(fp, "; 0=None, 1=NTSC, 2=xBR, 3=HqX, 4=2xSaI, 5=ScaleX\n");
		fprintf(fp, "filter=%d\n\n", conf.video_filter);
		fprintf(fp, "; Valid values are 1 to 4.\n");
		fprintf(fp, "scale_factor=%d\n\n", conf.video_scale_factor);
		fprintf(fp, "; 0=YUV, 1=RGB\n");
		fprintf(fp, "palette_mode=%d\n\n", conf.video_palette_mode);
		fprintf(fp, "; 0=Consumer, 1=Canonical, 2=Alternative\n");
		fprintf(fp, "decoder=%d\n\n", conf.video_decoder);
		fprintf(fp, "; Valid values are -100 to 100.\n");
		fprintf(fp, "brightness=%d\n", conf.video_brightness);
		fprintf(fp, "saturation=%d\n", conf.video_saturation);
		fprintf(fp, "contrast=%d\n\n", conf.video_contrast);
		fprintf(fp, "; Valid values are -45 to 45.\n");
		fprintf(fp, "hue=%d\n\n", conf.video_hue);
		fprintf(fp, "; 0=Composite, 1=S-Video, 2=RGB, 3=Custom\n");
		fprintf(fp, "ntsc_mode=%d\n\n", conf.video_ntsc_mode);
		fprintf(fp, "; Valid values are -100 to 100.\n");
		fprintf(fp, "ntsc_sharpness=%d\n", conf.video_ntsc_sharpness);
		fprintf(fp, "ntsc_resolution=%d\n", conf.video_ntsc_resolution);
		fprintf(fp, "ntsc_bleed=%d\n", conf.video_ntsc_bleed);
		fprintf(fp, "ntsc_artifacts=%d\n", conf.video_ntsc_artifacts);
		fprintf(fp, "ntsc_fringing=%d\n\n", conf.video_ntsc_fringing);
		fprintf(fp, "; 0=None, 1=Some, 2=All\n");
		fprintf(fp, "xbr_corner_rounding=%d\n\n", conf.video_xbr_corner_rounding);
		fprintf(fp, "; Valid values are 1 and 0.\n");
		fprintf(fp, "linear_filter=%d\n", conf.video_linear_filter);
		fprintf(fp, "tv_aspect=%d\n", conf.video_tv_aspect);
		fprintf(fp, "unmask_overscan=%d\n", conf.video_unmask_overscan);
		fprintf(fp, "fullscreen=%d\n", conf.video_fullscreen);
		fprintf(fp, "stretch_aspect=%d\n", conf.video_stretch_aspect);
		fprintf(fp, "unlimited_sprites=%d\n", conf.video_unlimited_sprites);
		fprintf(fp, "xbr_pixel_blending=%d\n", conf.video_xbr_pixel_blending);
		fprintf(fp, "\n"); // End of Section
		
		// Audio
		fprintf(fp, "[audio]\n");
		fprintf(fp, "; 0=SDL, 1=libao\n");
		fprintf(fp, "api=%d\n\n", conf.audio_api);
		fprintf(fp, "; Valid values are 1 and 0.\n");
		fprintf(fp, "stereo=%d\n\n", conf.audio_stereo);
		fprintf(fp, "; Valid values are 11025, 22050, 44100, 48000, and 96000.\n");
		fprintf(fp, "sample_rate=%d\n\n", conf.audio_sample_rate);
		fprintf(fp, "; Valid values are 0 to 100.\n");
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
		fprintf(fp, "; Base speed for NTSC in Frames per Second.\n");
		fprintf(fp, "speed=%d\n\n", conf.timing_speed);
		fprintf(fp, "; Alternate speed (slow down or fast forward)\n");
		fprintf(fp, "altspeed=%d\n\n", conf.timing_altspeed);
		fprintf(fp, "; Pulse turbo buttons every n frames. Minimum value is 2.\n");
		fprintf(fp, "turbopulse=%d\n\n", conf.timing_turbopulse);
		fprintf(fp, "; Valid values are 1 and 0.\n");
		fprintf(fp, "vsync=%d\n", conf.timing_vsync);
		fprintf(fp, "limiter=%d\n", conf.timing_limiter);
		fprintf(fp, "\n"); // End of Section
		
		// Misc
		fprintf(fp, "[misc]\n");
		fprintf(fp, "; 0=NTSC, 1=PAL, 2=Famicom, 3=Dendy\n");
		fprintf(fp, "default_system=%d\n\n", conf.misc_default_system);
		fprintf(fp, "; Valid values are 1 and 0.\n");
		fprintf(fp, "soft_patching=%d\n", conf.misc_soft_patching);
		//fprintf(fp, "suppress_screensaver=%d\n", conf.misc_suppress_screensaver);
		fprintf(fp, "genie_distortion=%d\n", conf.misc_genie_distortion);
		fprintf(fp, "disable_gui=%d\n", conf.misc_disable_gui);
		fprintf(fp, "config_pause=%d\n", conf.misc_config_pause);
		
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
	conf.video_ntsc_sharpness = 0; // -100 to 100
	conf.video_ntsc_resolution = 0; // -100 to 100
	conf.video_ntsc_bleed = 0; // -100 to 100
	conf.video_ntsc_artifacts = 0; // -100 to 100
	conf.video_ntsc_fringing = 0; // -100 to 100
	conf.video_xbr_corner_rounding = 0;
	conf.video_linear_filter = false;
	conf.video_tv_aspect = false;
	conf.video_unmask_overscan = false;
	conf.video_fullscreen = false;
	conf.video_stretch_aspect = false;
	conf.video_unlimited_sprites = false;
	conf.video_xbr_pixel_blending = false;
	
	// Audio
	conf.audio_api = 1;
	conf.audio_stereo = false;
	conf.audio_sample_rate = 44100;
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
	conf.timing_turbopulse = 3;
	conf.timing_vsync = true;
	conf.timing_limiter = true;
	
	// Misc
	conf.misc_default_system = 0;
	conf.misc_soft_patching = true;
	//conf.misc_suppress_screensaver = true;
	conf.misc_genie_distortion = false;
	conf.misc_disable_gui = false;
	conf.misc_config_pause = false;
	#ifdef _MINGW
	conf.misc_disable_gui = true; // Disable GUI for MinGW
	#endif
	#ifdef _APPLE
	conf.misc_config_pause = true; // Always pause on OS X
	#endif
}

static int config_match(void* user, const char* section, const char* name, const char* value) {
	// Match values from config file and populate live config
	settings_t* pconfig = (settings_t*)user;
	
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
	else if (MATCH("video", "ntsc_sharpness")) { pconfig->video_ntsc_sharpness = atoi(value); }
	else if (MATCH("video", "ntsc_resolution")) { pconfig->video_ntsc_resolution = atoi(value); }
	else if (MATCH("video", "ntsc_bleed")) { pconfig->video_ntsc_bleed = atoi(value); }
	else if (MATCH("video", "ntsc_artifacts")) { pconfig->video_ntsc_artifacts = atoi(value); }
	else if (MATCH("video", "ntsc_fringing")) { pconfig->video_ntsc_fringing = atoi(value); }
	else if (MATCH("video", "xbr_corner_rounding")) { pconfig->video_xbr_corner_rounding = atoi(value); }
	else if (MATCH("video", "linear_filter")) { pconfig->video_linear_filter = atoi(value); }
	else if (MATCH("video", "tv_aspect")) { pconfig->video_tv_aspect = atoi(value); }
	else if (MATCH("video", "unmask_overscan")) { pconfig->video_unmask_overscan = atoi(value); }
	else if (MATCH("video", "fullscreen")) { pconfig->video_fullscreen = atoi(value); }
	else if (MATCH("video", "stretch_aspect")) { pconfig->video_stretch_aspect = atoi(value); }
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
	else if (MATCH("timing", "turbopulse")) { pconfig->timing_turbopulse = atoi(value); }
	else if (MATCH("timing", "vsync")) { pconfig->timing_vsync = atoi(value); }
	else if (MATCH("timing", "limiter")) { pconfig->timing_limiter = atoi(value); }
    
	// Misc
	else if (MATCH("misc", "default_system")) { pconfig->misc_default_system = atoi(value); }
	else if (MATCH("misc", "soft_patching")) { pconfig->misc_soft_patching = atoi(value); }
	//else if (MATCH("misc", "suppress_screensaver")) { pconfig->misc_suppress_screensaver = atoi(value); }
	else if (MATCH("misc", "genie_distortion")) { pconfig->misc_genie_distortion = atoi(value); }
	else if (MATCH("misc", "disable_gui")) { pconfig->misc_disable_gui = atoi(value); }
	else if (MATCH("misc", "config_pause")) { pconfig->misc_config_pause = atoi(value); }
    
	else { return 0; }
	return 1;
}
