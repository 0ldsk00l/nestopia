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

#include <SDL.h>

#include "../main.h"
#include "../config.h"
#include "../video.h"

#include "gtkui.h"
#include "gtkui_callbacks.h"

extern settings_t conf;
extern bool playing, kbactivate, confrunning;

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
	if (playing) {
		video_init();
	}
	else { video_set_dimensions(); }
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
	if (playing) {
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
	
	if (playing) {
		nst_pause();
		nst_play();
	}
}

void gtkui_cb_audio_stereo(GtkToggleButton *togglebutton, gpointer userdata) {
	// Toggle Stereo
	conf.audio_stereo = gtk_toggle_button_get_active(togglebutton);
	
	if (playing) {
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

void gtkui_cb_timing_altspeed(GtkRange *range, gpointer userdata) {
	// Set altspeed
	conf.timing_altspeed = (int)gtk_range_get_value(range);
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

void gtkui_cb_misc_disable_gui(GtkToggleButton *togglebutton, gpointer userdata) {
	// Enable or Disable the GUI
	conf.misc_disable_gui = gtk_toggle_button_get_active(togglebutton);
}

void gtkui_cb_misc_config_pause(GtkToggleButton *togglebutton, gpointer userdata) {
	// Enable or Disable the GUI
	conf.misc_config_pause = gtk_toggle_button_get_active(togglebutton);
}

//// Key Translation ////

// This function is based on code from FCEUX
unsigned int gtkui_cb_translate_gdk_sdl(int gdk_keyval) {
	// Translate GDK keys to SDL keys
	
	if (!(gdk_keyval & 0xFF00))	{
		gdk_keyval = tolower(gdk_keyval);
		return gdk_keyval;
	}
	
	if (gdk_keyval & 0xFFFF0000) {
		fprintf(stderr, "Unhandled extended key: 0x%08X\n", gdk_keyval);
		return 0;
	}
	
	// Non-ASCII symbol.
	static const SDL_Keycode gdk_to_sdl_table[0x100] = {
		// 0x00 - 0x0F
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		SDLK_BACKSPACE, SDLK_TAB, SDLK_RETURN, SDLK_CLEAR,
		0x0000, SDLK_RETURN, 0x0000, 0x0000,
		
		// 0x10 - 0x1F
		0x0000, 0x0000, 0x0000, SDLK_PAUSE,
		SDLK_SCROLLLOCK, SDLK_SYSREQ, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, SDLK_ESCAPE,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x20 - 0x2F
		SDLK_APPLICATION, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x30 - 0x3F [Japanese keys]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x40 - 0x4F [unused]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x50 - 0x5F
		SDLK_HOME, SDLK_LEFT, SDLK_UP, SDLK_RIGHT,
		SDLK_DOWN, SDLK_PAGEUP, SDLK_PAGEDOWN, SDLK_END,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x60 - 0x6F
		0x0000, SDLK_PRINTSCREEN, 0x0000, SDLK_INSERT,
		SDLK_UNDO, 0x0000, 0x0000, SDLK_MENU,		
		0x0000, SDLK_HELP, SDLK_PAUSE, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x70 - 0x7F [mostly unused, except for Alt Gr and Num Lock]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, SDLK_MODE, SDLK_NUMLOCKCLEAR,
		
		// 0x80 - 0x8F [mostly unused, except for some numeric keypad keys]
		SDLK_KP_5, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, SDLK_KP_ENTER, 0x0000, 0x0000,
		
		// 0x90 - 0x9F
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, SDLK_KP_7, SDLK_KP_4, SDLK_KP_8,
		SDLK_KP_6, SDLK_KP_2, SDLK_KP_9, SDLK_KP_3,
		SDLK_KP_1, SDLK_KP_5, SDLK_KP_0, SDLK_KP_PERIOD,
		
		// 0xA0 - 0xAF
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, SDLK_KP_MULTIPLY, SDLK_KP_PLUS,
		0x0000, SDLK_KP_MINUS, SDLK_KP_PERIOD, SDLK_KP_DIVIDE,
		
		// 0xB0 - 0xBF
		SDLK_KP_0, SDLK_KP_1, SDLK_KP_2, SDLK_KP_3,
		SDLK_KP_4, SDLK_KP_5, SDLK_KP_6, SDLK_KP_7,
		SDLK_KP_8, SDLK_KP_9, 0x0000, 0x0000,
		0x0000, SDLK_KP_EQUALS, SDLK_F1, SDLK_F2,
		
		// 0xC0 - 0xCF
		SDLK_F3, SDLK_F4, SDLK_F5, SDLK_F6,
		SDLK_F7, SDLK_F8, SDLK_F9, SDLK_F10,
		SDLK_F11, SDLK_F12, SDLK_F13, SDLK_F14,
		SDLK_F15, 0x0000, 0x0000, 0x0000,
		
		// 0xD0 - 0xDF [L* and R* function keys]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0xE0 - 0xEF
		0x0000, SDLK_LSHIFT, SDLK_RSHIFT, SDLK_LCTRL,
		SDLK_RCTRL, SDLK_CAPSLOCK, 0x0000, SDLK_LGUI,
		SDLK_RGUI, SDLK_LALT, SDLK_RALT, SDLK_LGUI,
		SDLK_RGUI, 0x0000, 0x0000, 0x0000,
		
		// 0xF0 - 0xFF [mostly unused, except for Delete]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, SDLK_DELETE,		
	};
	
	SDL_Keycode sdl_keycode = gdk_to_sdl_table[gdk_keyval & 0xFF];
	
	return sdl_keycode;
}

int gtkui_cb_convert_key(GtkWidget *grab, GdkEventKey *event, gpointer userdata) {
	// Convert GDK events to SDL events
	
	SDL_Event sdlevent;
	SDL_Keycode sdlkeycode;
	int keystate;
	
	if (userdata) {
		kbactivate = true;
		return FALSE;
	}
	else { kbactivate = false; }
	
	switch (event->type) {
		case GDK_KEY_PRESS:
			sdlevent.type = SDL_KEYDOWN;
			sdlevent.key.state = SDL_PRESSED;
			keystate = 1;
			break;
		
		case GDK_KEY_RELEASE:
			sdlevent.type = SDL_KEYUP;
			sdlevent.key.state = SDL_RELEASED;
			keystate = 0;
			break;
		
		default:
			fprintf(stderr, "Unhandled GDK event type: %d", event->type);
			return FALSE;
			break;
	}
	
	sdlkeycode = (SDL_Keycode)gtkui_cb_translate_gdk_sdl(event->keyval);
	sdlevent.key.keysym.sym = sdlkeycode;
	sdlevent.key.keysym.scancode = SDL_GetScancodeFromKey(sdlevent.key.keysym.sym);
	
	if (sdlkeycode != 0 && (playing || confrunning)) {
		SDL_PushEvent(&sdlevent);
		
		const Uint8 *statebuffer = SDL_GetKeyboardState(NULL);
		Uint8 *state = (Uint8*)statebuffer;
		state[SDL_GetScancodeFromKey(sdlkeycode)] = keystate;
	}
	// Allow GTK+ to process this key.
	return FALSE;
}

int gtkui_cb_convert_mouse(GtkDrawingArea *area, GdkEventButton *event, gpointer userdata) {
	// Convert GDK mouse clicks to SDL mouse clicks
	SDL_Event sdlevent;
	
	switch(event->type) {
		case GDK_BUTTON_PRESS:
			sdlevent.type = SDL_MOUSEBUTTONDOWN;
			sdlevent.button.state = SDL_PRESSED;
			break;
		
		case GDK_BUTTON_RELEASE:
			sdlevent.type = SDL_MOUSEBUTTONUP;
			sdlevent.button.state = SDL_RELEASED;
			break;
		
		default: break;
	}
	
	sdlevent.button.button = event->button == 1 ? SDL_BUTTON_LEFT : SDL_BUTTON_RIGHT;
	sdlevent.button.x = (int)event->x;
	sdlevent.button.y = (int)event->y;
	
	SDL_PushEvent(&sdlevent);
	
	return FALSE;
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
