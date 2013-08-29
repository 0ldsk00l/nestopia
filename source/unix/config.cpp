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

#include "config.h"

settings *conf;
GKeyFile *keyfile;
static GKeyFileFlags flags;
static gsize length;

char confpath[256];
	
void read_config_file() {
	
	char *homedir;

	homedir = getenv("HOME");
	snprintf(confpath, sizeof(confpath), "%s/.nestopia/nestopia.conf", homedir);
	
	keyfile = g_key_file_new();
	
	flags = G_KEY_FILE_KEEP_COMMENTS;
	
	// Set aside memory for the settings
	conf = g_slice_new(settings);
	
	if (g_key_file_load_from_file(keyfile, confpath, flags, NULL)) {
		// Video
		conf->video_renderer = g_key_file_get_integer(keyfile, "video", "renderer", NULL);
		conf->video_filter = g_key_file_get_integer(keyfile, "video", "filter", NULL);
		conf->video_scale_factor = g_key_file_get_integer(keyfile, "video", "scale_factor", NULL);
		conf->video_ntsc_mode = g_key_file_get_integer(keyfile, "video", "ntsc_mode", NULL);
		conf->video_xbr_corner_rounding = g_key_file_get_integer(keyfile, "video", "xbr_corner_rounding", NULL);
		
		conf->video_xbr_pixel_blending = g_key_file_get_boolean(keyfile, "video", "xbr_pixel_blending", NULL);
		conf->video_tv_aspect = g_key_file_get_boolean(keyfile, "video", "tv_aspect", NULL);
		conf->video_mask_overscan = g_key_file_get_boolean(keyfile, "video", "mask_overscan", NULL);
		conf->video_fullscreen = g_key_file_get_boolean(keyfile, "video", "fullscreen", NULL);
		conf->video_stretch_fullscreen = g_key_file_get_boolean(keyfile, "video", "stretch_fullscreen", NULL);
		conf->video_unlimited_sprites = g_key_file_get_boolean(keyfile, "video", "unlimited_sprites", NULL);
	
		// Audio
		conf->audio_api = g_key_file_get_integer(keyfile, "audio", "api", NULL);
		conf->audio_sample_rate = g_key_file_get_integer(keyfile, "audio", "sample_rate", NULL);
		conf->audio_volume = g_key_file_get_integer(keyfile, "audio", "volume", NULL);
		conf->audio_surround_multiplier = g_key_file_get_integer(keyfile, "audio", "surround_multiplier", NULL);
		
		conf->audio_surround = g_key_file_get_boolean(keyfile, "audio", "surround", NULL);
		conf->audio_stereo = g_key_file_get_boolean(keyfile, "audio", "stereo", NULL);
		conf->audio_stereo_exciter = g_key_file_get_boolean(keyfile, "audio", "stereo_exciter", NULL);
		
		// Misc
		conf->misc_video_region = g_key_file_get_integer(keyfile, "misc", "video_region", NULL);
		conf->misc_default_system = g_key_file_get_integer(keyfile, "misc", "default_system", NULL);
		conf->misc_soft_patching = g_key_file_get_boolean(keyfile, "misc", "soft_patching", NULL);
		conf->misc_suppress_screensaver = g_key_file_get_boolean(keyfile, "misc", "suppress_screensaver", NULL);
		conf->misc_disable_gui = g_key_file_get_boolean(keyfile, "misc", "disable_gui", NULL);
	}
	else {
		fprintf(stderr, "Failed to read config file %s: Using defaults.\n", confpath);
		set_default_config();
	}
}

void write_config_file() {
	
	// Video
	g_key_file_set_integer(keyfile, "video", "renderer", conf->video_renderer);
	g_key_file_set_integer(keyfile, "video", "filter", conf->video_filter);
	g_key_file_set_integer(keyfile, "video", "scale_factor", conf->video_scale_factor);
	g_key_file_set_integer(keyfile, "video", "ntsc_mode", conf->video_ntsc_mode);
	g_key_file_set_integer(keyfile, "video", "xbr_corner_rounding", conf->video_xbr_corner_rounding);
	
	g_key_file_set_boolean(keyfile, "video", "xbr_pixel_blending", conf->video_xbr_pixel_blending);
	g_key_file_set_boolean(keyfile, "video", "tv_aspect", conf->video_tv_aspect);
	g_key_file_set_boolean(keyfile, "video", "mask_overscan", conf->video_mask_overscan);
	g_key_file_set_boolean(keyfile, "video", "fullscreen", conf->video_fullscreen);
	g_key_file_set_boolean(keyfile, "video", "stretch_fullscreen", conf->video_stretch_fullscreen);
	g_key_file_set_boolean(keyfile, "video", "unlimited_sprites", conf->video_unlimited_sprites);
	
	// Audio
	g_key_file_set_integer(keyfile, "audio", "api", conf->audio_api);
	g_key_file_set_integer(keyfile, "audio", "sample_rate", conf->audio_sample_rate);
	g_key_file_set_integer(keyfile, "audio", "volume", conf->audio_volume);
	g_key_file_set_integer(keyfile, "audio", "surround_multiplier", conf->audio_surround_multiplier);
	
	g_key_file_set_boolean(keyfile, "audio", "surround", conf->audio_surround);
	g_key_file_set_boolean(keyfile, "audio", "stereo", conf->audio_stereo);
	g_key_file_set_boolean(keyfile, "audio", "stereo_exciter", conf->audio_stereo_exciter);
	
	// Misc
	g_key_file_set_integer(keyfile, "misc", "video_region", conf->misc_video_region);
	g_key_file_set_integer(keyfile, "misc", "default_system", conf->misc_default_system);
	g_key_file_set_boolean(keyfile, "misc", "soft_patching", conf->misc_soft_patching);
	g_key_file_set_boolean(keyfile, "misc", "suppress_screensaver", conf->misc_suppress_screensaver);
	g_key_file_set_boolean(keyfile, "misc", "disable_gui", conf->misc_disable_gui);
	
	FILE *fp = fopen(confpath, "w");
	if (fp != NULL)	{
		fputs(g_key_file_to_data(keyfile, &length, NULL), fp);
		fclose(fp);
	}
	
	g_slice_free(settings, conf);
	g_key_file_free(keyfile);
}

void set_default_config() {
	
	// Video
	conf->video_renderer = 1;
	conf->video_filter = 0;
	conf->video_scale_factor = 2;
	conf->video_ntsc_mode = 0;
	conf->video_xbr_corner_rounding = 0;
	
	conf->video_xbr_pixel_blending = true;
	conf->video_tv_aspect = false;
	conf->video_mask_overscan = false;
	conf->video_fullscreen = false;
	conf->video_stretch_fullscreen = true;
	conf->video_unlimited_sprites = true;
	
	// Audio
	conf->audio_api = 0;
	conf->audio_sample_rate = 48000;
	conf->audio_volume = 85;
	conf->audio_surround_multiplier = 50;
	
	conf->audio_surround = false;
	conf->audio_stereo = false;
	conf->audio_stereo_exciter = false;
	
	// Misc
	conf->misc_video_region = 0;
	conf->misc_default_system = 0;
	conf->misc_soft_patching = true;
	conf->misc_suppress_screensaver = true;
	conf->misc_disable_gui = false;
}
