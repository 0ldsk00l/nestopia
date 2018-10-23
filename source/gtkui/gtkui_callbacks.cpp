/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2017 R. Danbrook
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

#include <SDL.h>

#include "nstcommon.h"
#include "config.h"
#include "video.h"
#include "input.h"

#include "gtkui.h"
#include "gtkui_callbacks.h"

extern bool kbactivate, confrunning;
extern int nst_quit;

//// Menu ////

void gtkui_cb_reset(GtkWidget *reset, int hard) {
	// Reset the NES from the GUI
	nst_reset(hard);
}

void gtkui_cb_nothing() {
	// Do nothing
}

void gtkui_cb_video_refresh() {
	// Refresh the Video output after changes
	if (nst_playing()) { video_init(); }
	gtkui_resize();
}

// Video //

void gtkui_cb_video_filter(GtkComboBox *combobox, gpointer userdata) {
	// Change the video filter
	conf.video_filter = gtk_combo_box_get_active(combobox);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_scale(GtkComboBox *combobox, gpointer userdata) {
	// Change the scale factor
	conf.video_scale_factor = gtk_combo_box_get_active(combobox) + 1;
	
	// The scalex filter only allows 3x scale and crashes otherwise
	if (conf.video_filter == 5 && conf.video_scale_factor == 4) {
		conf.video_scale_factor = 3;
	}
	
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_ntscmode(GtkComboBox *combobox, gpointer userdata) {
	// Change the NTSC Mode
	conf.video_ntsc_mode = gtk_combo_box_get_active(combobox);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_xbrrounding(GtkComboBox *combobox, gpointer userdata) {
	// Set xBR corner rounding parameters
	conf.video_xbr_corner_rounding = gtk_combo_box_get_active(combobox);
	gtkui_cb_video_refresh();
	video_toggle_filterupdate();
}

void gtkui_cb_video_xbrpixblend(GtkToggleButton *togglebutton, gpointer userdata) {
	// Set xBR pixel blending parameters
	conf.video_xbr_pixel_blending = gtk_toggle_button_get_active(togglebutton);
	gtkui_cb_video_refresh();
	video_toggle_filterupdate();
}

void gtkui_cb_video_linear_filter(GtkToggleButton *togglebutton, gpointer userdata) {
	// Set linear filter
	conf.video_linear_filter = gtk_toggle_button_get_active(togglebutton);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_tv_aspect(GtkToggleButton *togglebutton, gpointer userdata) {
	// Set TV aspect ratio
	conf.video_tv_aspect = gtk_toggle_button_get_active(togglebutton);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_unmask_overscan(GtkToggleButton *togglebutton, gpointer userdata) {
	// Set overscan mask
	conf.video_unmask_overscan = gtk_toggle_button_get_active(togglebutton);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_stretch_aspect(GtkToggleButton *togglebutton, gpointer userdata) {
	// Set aspect ratio stretching/preservation
	conf.video_stretch_aspect = gtk_toggle_button_get_active(togglebutton);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_unlimited_sprites(GtkToggleButton *togglebutton, gpointer userdata) {
	// Set sprite limit
	conf.video_unlimited_sprites = gtk_toggle_button_get_active(togglebutton);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_palette(GtkComboBox *combobox, gpointer userdata) {
	// Change the video palette
	conf.video_palette_mode = gtk_combo_box_get_active(combobox);
	gtkui_cb_video_refresh();
	// this doesn't work unless there's a restart - fix
}

void gtkui_cb_video_decoder(GtkComboBox *combobox, gpointer userdata) {
	// Change the YUV Decoder
	conf.video_decoder = gtk_combo_box_get_active(combobox);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_brightness(GtkRange *range, gpointer userdata) {
	// Change video brightness
	conf.video_brightness = (int)gtk_range_get_value(range);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_saturation(GtkRange *range, gpointer userdata) {
	// Change video saturation
	conf.video_saturation = (int)gtk_range_get_value(range);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_contrast(GtkRange *range, gpointer userdata) {
	// Change video contrast
	conf.video_contrast = (int)gtk_range_get_value(range);
	gtkui_cb_video_refresh();
}

void gtkui_cb_video_hue(GtkRange *range, gpointer userdata) {
	// Change video hue
	conf.video_hue = (int)gtk_range_get_value(range);
	gtkui_cb_video_refresh();
}

// Audio //

void gtkui_cb_audio_api(GtkComboBox *combobox, gpointer userdata) {
	// Change the Audio API
	if (nst_playing()) {
		nst_pause();
		conf.audio_api = gtk_combo_box_get_active(combobox);
		nst_play();
	}
	else { conf.audio_api = gtk_combo_box_get_active(combobox); }
}

void gtkui_cb_audio_samplerate(GtkComboBox *combobox, gpointer userdata) {
	// Change the Sample Rate
	switch (gtk_combo_box_get_active(combobox)) {
		case 0:
			conf.audio_sample_rate = 11025;
			break;
		case 1:
			conf.audio_sample_rate = 22050;
			break;
		case 2:
			conf.audio_sample_rate = 44100;
			break;
		case 3:
			conf.audio_sample_rate = 48000;
			break;
		case 4:
			conf.audio_sample_rate = 96000;
			break;
		default:
			conf.audio_sample_rate = 44100;
			break;
	}
	
	if (nst_playing()) {
		nst_pause();
		nst_play();
	}
}

void gtkui_cb_audio_stereo(GtkToggleButton *togglebutton, gpointer userdata) {
	// Toggle Stereo
	conf.audio_stereo = gtk_toggle_button_get_active(togglebutton);
	
	if (nst_playing()) {
		nst_pause();
		nst_play();
	}
}

//// Input ////

void gtkui_cb_input_turbopulse(GtkRange *range, gpointer userdata) {
	// Change turbo pulse
	conf.timing_turbopulse = (int)gtk_range_get_value(range);
}

//// Misc ////

void gtkui_cb_misc_default_system(GtkComboBox *combobox, gpointer userdata) {
	// Select the default system
	conf.misc_default_system = gtk_combo_box_get_active(combobox);
}

void gtkui_cb_misc_power_state(GtkComboBox *combobox, gpointer userdata) {
	// Select the default system
	conf.misc_power_state = gtk_combo_box_get_active(combobox);
}

void gtkui_cb_timing_ffspeed(GtkRange *range, gpointer userdata) {
	// Set Fast-Forward Speed
	conf.timing_ffspeed = (int)gtk_range_get_value(range);
}

void gtkui_cb_timing_vsync(GtkToggleButton *togglebutton, gpointer userdata) {
	// Toggle vsync
	conf.timing_vsync = gtk_toggle_button_get_active(togglebutton);
}

void gtkui_cb_timing_limiter(GtkToggleButton *togglebutton, gpointer userdata) {
	// Set the limiter on or off
	conf.timing_limiter = gtk_toggle_button_get_active(togglebutton);
}

void gtkui_cb_misc_soft_patching(GtkToggleButton *togglebutton, gpointer userdata) {
	// Enable or Disable automatic soft patching
	conf.misc_soft_patching = gtk_toggle_button_get_active(togglebutton);
}

void gtkui_cb_misc_genie_distortion(GtkToggleButton *togglebutton, gpointer userdata) {
	// Enable or Disable Game Genie Sound Distortion
	conf.misc_genie_distortion = gtk_toggle_button_get_active(togglebutton);
}

void gtkui_cb_misc_disable_cursor(GtkToggleButton *togglebutton, gpointer userdata) {
	// Enable or Disable the Cursor
	conf.misc_disable_cursor = gtk_toggle_button_get_active(togglebutton);
	if (!nst_quit) { gtkui_play(); }
}

void gtkui_cb_misc_disable_cursor_special(GtkToggleButton *togglebutton, gpointer userdata) {
	// Enable or Disable Special Cursors
	conf.misc_disable_cursor_special = gtk_toggle_button_get_active(togglebutton);
	if (!nst_quit) { gtkui_play(); }
}

void gtkui_cb_misc_config_pause(GtkToggleButton *togglebutton, gpointer userdata) {
	// Pause GUI when configuration window is open
	conf.misc_config_pause = gtk_toggle_button_get_active(togglebutton);
}

void gtkui_cb_misc_overclock(GtkToggleButton *togglebutton, gpointer userdata) {
	// Enable or Disable Core Overclocking
	conf.misc_overclock = gtk_toggle_button_get_active(togglebutton);
	nst_set_overclock();
}

void gtkui_drag_data(GtkWidget *widget, GdkDragContext *dragcontext, gint x, gint y, GtkSelectionData *seldata, guint info, guint time, gpointer data) {
	// Handle the Drag and Drop
	if ((widget == NULL) || (dragcontext == NULL) || (seldata == NULL)) {	return;	}
	
	if (info == 0) {
		gchar *fileuri = (gchar*)gtk_selection_data_get_data(seldata);
		gchar *filename = g_filename_from_uri(fileuri, NULL, NULL);
		
		// Dirty hack. g_filename_from_uri adds a \r\n to the string
		size_t ln = strlen(filename) - 2;
		if (filename[ln] == '\r') { filename[ln] = '\0'; }
		
		nst_load(filename);
	}
}
