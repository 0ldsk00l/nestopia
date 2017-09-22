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

#include "../main.h"
#include "../config.h"
#include "../input.h"
#include "../video.h"
#include "../audio.h"
#include "../ini.h"

#include "gtkui.h"
#include "gtkui_input.h"

static inputsettings_t inputconf;
static gkeys_t ui;
gpad_t pad[NUMGAMEPADS];

static char inputconfpath[256];

//extern settings_t conf;
extern nstpaths_t nstpaths;
extern Input::Controllers *cNstPads;

static int gtkui_input_config_match(void* user, const char* section, const char* name, const char* value) {
	// Match values from input config file and populate live config
	inputsettings_t* pconfig = (inputsettings_t*)user;
	
	// User Interface
	if (MATCH("ui", "qsave1")) { pconfig->qsave1 = strdup(value); }
	else if (MATCH("ui", "qsave2")) { pconfig->qsave2 = strdup(value); }
	else if (MATCH("ui", "qload1")) { pconfig->qload1 = strdup(value); }
	else if (MATCH("ui", "qload2")) { pconfig->qload2 = strdup(value); }
	else if (MATCH("ui", "screenshot")) { pconfig->screenshot = strdup(value); }
	else if (MATCH("ui", "fdsflip")) { pconfig->fdsflip = strdup(value); }
	else if (MATCH("ui", "fdsswitch")) { pconfig->fdsswitch = strdup(value); }
	else if (MATCH("ui", "insertcoin1")) { pconfig->insertcoin1 = strdup(value); }
	else if (MATCH("ui", "insertcoin2")) { pconfig->insertcoin2 = strdup(value); }
	else if (MATCH("ui", "reset")) { pconfig->reset = strdup(value); }
	else if (MATCH("ui", "ffspeed")) { pconfig->ffspeed = strdup(value); }
	else if (MATCH("ui", "rwstart")) { pconfig->rwstart = strdup(value); }
	else if (MATCH("ui", "rwstop")) { pconfig->rwstop = strdup(value); }
	else if (MATCH("ui", "fullscreen")) { pconfig->fullscreen = strdup(value); }
	else if (MATCH("ui", "filter")) { pconfig->filter = strdup(value); }
	else if (MATCH("ui", "scalefactor")) { pconfig->scalefactor = strdup(value); }
	
	// Player 1
	else if (MATCH("gamepad1", "kb_u")) { pconfig->kb_p1u = strdup(value); }
	else if (MATCH("gamepad1", "kb_d")) { pconfig->kb_p1d = strdup(value); }
	else if (MATCH("gamepad1", "kb_l")) { pconfig->kb_p1l = strdup(value); }
	else if (MATCH("gamepad1", "kb_r")) { pconfig->kb_p1r = strdup(value); }
	else if (MATCH("gamepad1", "kb_select")) { pconfig->kb_p1select = strdup(value); }
	else if (MATCH("gamepad1", "kb_start")) { pconfig->kb_p1start = strdup(value); }
	else if (MATCH("gamepad1", "kb_a")) { pconfig->kb_p1a = strdup(value); }
	else if (MATCH("gamepad1", "kb_b")) { pconfig->kb_p1b = strdup(value); }
	else if (MATCH("gamepad1", "kb_ta")) { pconfig->kb_p1ta = strdup(value); }
	else if (MATCH("gamepad1", "kb_tb")) { pconfig->kb_p1tb = strdup(value); }
	
	// Player 2
	else if (MATCH("gamepad2", "kb_u")) { pconfig->kb_p2u = strdup(value); }
	else if (MATCH("gamepad2", "kb_d")) { pconfig->kb_p2d = strdup(value); }
	else if (MATCH("gamepad2", "kb_l")) { pconfig->kb_p2l = strdup(value); }
	else if (MATCH("gamepad2", "kb_r")) { pconfig->kb_p2r = strdup(value); }
	else if (MATCH("gamepad2", "kb_select")) { pconfig->kb_p2select = strdup(value); }
	else if (MATCH("gamepad2", "kb_start")) { pconfig->kb_p2start = strdup(value); }
	else if (MATCH("gamepad2", "kb_a")) { pconfig->kb_p2a = strdup(value); }
	else if (MATCH("gamepad2", "kb_b")) { pconfig->kb_p2b = strdup(value); }
	else if (MATCH("gamepad2", "kb_ta")) { pconfig->kb_p2ta = strdup(value); }
	else if (MATCH("gamepad2", "kb_tb")) { pconfig->kb_p2tb = strdup(value); }
	
	else { return 0; }
    return 1;
}

void gtkui_input_set_default() {
	// Set the default input for GTK+
	
	// Gamepads
	pad[0].u = GDK_KEY_Up;
	pad[0].d = GDK_KEY_Down;
	pad[0].l = GDK_KEY_Left;
	pad[0].r = GDK_KEY_Right;
	pad[0].select = GDK_KEY_Shift_R;
	pad[0].start = GDK_KEY_Return;
	pad[0].a = GDK_KEY_z;
	pad[0].b = GDK_KEY_a;
	pad[0].ta = GDK_KEY_x;
	pad[0].tb = GDK_KEY_s;
	
	pad[1].u = GDK_KEY_i;
	pad[1].d = GDK_KEY_k;
	pad[1].l = GDK_KEY_j;
	pad[1].r = GDK_KEY_l;
	pad[1].select = GDK_KEY_Shift_L;
	pad[1].start = GDK_KEY_Control_L;
	pad[1].a = GDK_KEY_m;
	pad[1].b = GDK_KEY_n;
	pad[1].ta = GDK_KEY_b;
	pad[1].tb = GDK_KEY_v;
	
	// User Interface
	ui.qsave1 = GDK_KEY_F5;
	ui.qsave2 = GDK_KEY_F6;
	ui.qload1 = GDK_KEY_F7;
	ui.qload2 = GDK_KEY_F8;
	ui.screenshot = GDK_KEY_F9;
	ui.fdsflip = GDK_KEY_F3;
	ui.fdsswitch = GDK_KEY_F4;
	ui.insertcoin1 = GDK_KEY_F1;
	ui.insertcoin2 = GDK_KEY_F2;
	ui.reset = GDK_KEY_F12;
	ui.ffspeed = GDK_KEY_grave;
	ui.rwstart = GDK_KEY_BackSpace;
	ui.rwstop = GDK_KEY_backslash;
	ui.fullscreen = GDK_KEY_f;
	ui.filter = GDK_KEY_t;
	ui.scalefactor = GDK_KEY_g;
}

void gtkui_input_config_read() {
	// Read the input config file
	snprintf(inputconfpath, sizeof(inputconfpath), "%sgtkinput.conf", nstpaths.nstdir);
	if (ini_parse(inputconfpath, gtkui_input_config_match, &inputconf) < 0) {
		fprintf(stderr, "Failed to load input config file %s: Using defaults.\n", inputconfpath);
	}
	else {
		// Map the input settings from the config file
		
		// User Interface
		ui.qsave1 = gdk_keyval_from_name(inputconf.qsave1);
		ui.qsave2 = gdk_keyval_from_name(inputconf.qsave2);
		ui.qload1 = gdk_keyval_from_name(inputconf.qload1);
		ui.qload2 = gdk_keyval_from_name(inputconf.qload2);
		ui.screenshot = gdk_keyval_from_name(inputconf.screenshot);
		ui.fdsflip = gdk_keyval_from_name(inputconf.fdsflip);
		ui.fdsswitch = gdk_keyval_from_name(inputconf.fdsswitch);
		ui.insertcoin1 = gdk_keyval_from_name(inputconf.insertcoin1);
		ui.insertcoin2 = gdk_keyval_from_name(inputconf.insertcoin2);
		ui.reset = gdk_keyval_from_name(inputconf.reset);
		ui.ffspeed = gdk_keyval_from_name(inputconf.ffspeed);
		ui.rwstart = gdk_keyval_from_name(inputconf.rwstart);
		ui.rwstop = gdk_keyval_from_name(inputconf.rwstop);
		ui.fullscreen = gdk_keyval_from_name(inputconf.fullscreen);
		ui.filter = gdk_keyval_from_name(inputconf.filter);
		ui.scalefactor = gdk_keyval_from_name(inputconf.scalefactor);
		
		// Player 1
		pad[0].u = gdk_keyval_from_name(inputconf.kb_p1u);
		pad[0].d = gdk_keyval_from_name(inputconf.kb_p1d);
		pad[0].l = gdk_keyval_from_name(inputconf.kb_p1l);
		pad[0].r = gdk_keyval_from_name(inputconf.kb_p1r);
		pad[0].select = gdk_keyval_from_name(inputconf.kb_p1select);
		pad[0].start = gdk_keyval_from_name(inputconf.kb_p1start);
		pad[0].a = gdk_keyval_from_name(inputconf.kb_p1a);
		pad[0].b = gdk_keyval_from_name(inputconf.kb_p1b);
		pad[0].ta = gdk_keyval_from_name(inputconf.kb_p1ta);
		pad[0].tb = gdk_keyval_from_name(inputconf.kb_p1tb);
		
		// Player 2
		pad[1].u = gdk_keyval_from_name(inputconf.kb_p2u);
		pad[1].d = gdk_keyval_from_name(inputconf.kb_p2d);
		pad[1].l = gdk_keyval_from_name(inputconf.kb_p2l);
		pad[1].r = gdk_keyval_from_name(inputconf.kb_p2r);
		pad[1].select = gdk_keyval_from_name(inputconf.kb_p2select);
		pad[1].start = gdk_keyval_from_name(inputconf.kb_p2start);
		pad[1].a = gdk_keyval_from_name(inputconf.kb_p2a);
		pad[1].b = gdk_keyval_from_name(inputconf.kb_p2b);
		pad[1].ta = gdk_keyval_from_name(inputconf.kb_p2ta);
		pad[1].tb = gdk_keyval_from_name(inputconf.kb_p2tb);
	}
}

void gtkui_input_config_write() {
	// Write out the input configuration file
	
	FILE *fp = fopen(inputconfpath, "w");
	if (fp != NULL)	{
		fprintf(fp, "; Nestopia UE Input Configuration File\n\n");
		fprintf(fp, "; Values for keyboard input are these values with the GDK_KEY_ prefix removed:\n; https://git.gnome.org/browse/gtk+/plain/gdk/gdkkeysyms.h\n\n");
		
		fprintf(fp, "[ui]\n");
		fprintf(fp, "qsave1=%s\n", gdk_keyval_name(ui.qsave1));
		fprintf(fp, "qsave2=%s\n", gdk_keyval_name(ui.qsave2));
		fprintf(fp, "qload1=%s\n", gdk_keyval_name(ui.qload1));
		fprintf(fp, "qload2=%s\n", gdk_keyval_name(ui.qload2));
		fprintf(fp, "screenshot=%s\n", gdk_keyval_name(ui.screenshot));
		fprintf(fp, "fdsflip=%s\n", gdk_keyval_name(ui.fdsflip));
		fprintf(fp, "fdsswitch=%s\n", gdk_keyval_name(ui.fdsswitch));
		fprintf(fp, "insertcoin1=%s\n", gdk_keyval_name(ui.insertcoin1));
		fprintf(fp, "insertcoin2=%s\n", gdk_keyval_name(ui.insertcoin2));
		fprintf(fp, "reset=%s\n", gdk_keyval_name(ui.reset));
		fprintf(fp, "ffspeed=%s\n", gdk_keyval_name(ui.ffspeed));
		fprintf(fp, "rwstart=%s\n", gdk_keyval_name(ui.rwstart));
		fprintf(fp, "rwstop=%s\n", gdk_keyval_name(ui.rwstop));
		fprintf(fp, "fullscreen=%s\n", gdk_keyval_name(ui.fullscreen));
		fprintf(fp, "filter=%s\n", gdk_keyval_name(ui.filter));
		fprintf(fp, "scalefactor=%s\n", gdk_keyval_name(ui.scalefactor));
		fprintf(fp, "\n"); // End of Section
		
		fprintf(fp, "[gamepad1]\n");
		fprintf(fp, "kb_u=%s\n", gdk_keyval_name(pad[0].u));
		fprintf(fp, "kb_d=%s\n", gdk_keyval_name(pad[0].d));
		fprintf(fp, "kb_l=%s\n", gdk_keyval_name(pad[0].l));
		fprintf(fp, "kb_r=%s\n", gdk_keyval_name(pad[0].r));
		fprintf(fp, "kb_select=%s\n", gdk_keyval_name(pad[0].select));
		fprintf(fp, "kb_start=%s\n", gdk_keyval_name(pad[0].start));
		fprintf(fp, "kb_a=%s\n", gdk_keyval_name(pad[0].a));
		fprintf(fp, "kb_b=%s\n", gdk_keyval_name(pad[0].b));
		fprintf(fp, "kb_ta=%s\n", gdk_keyval_name(pad[0].ta));
		fprintf(fp, "kb_tb=%s\n", gdk_keyval_name(pad[0].tb));
		fprintf(fp, "\n"); // End of Section
		
		fprintf(fp, "[gamepad2]\n");
		fprintf(fp, "kb_u=%s\n", gdk_keyval_name(pad[1].u));
		fprintf(fp, "kb_d=%s\n", gdk_keyval_name(pad[1].d));
		fprintf(fp, "kb_l=%s\n", gdk_keyval_name(pad[1].l));
		fprintf(fp, "kb_r=%s\n", gdk_keyval_name(pad[1].r));
		fprintf(fp, "kb_select=%s\n", gdk_keyval_name(pad[1].select));
		fprintf(fp, "kb_start=%s\n", gdk_keyval_name(pad[1].start));
		fprintf(fp, "kb_a=%s\n", gdk_keyval_name(pad[1].a));
		fprintf(fp, "kb_b=%s\n", gdk_keyval_name(pad[1].b));
		fprintf(fp, "kb_ta=%s\n", gdk_keyval_name(pad[1].ta));
		fprintf(fp, "kb_tb=%s\n", gdk_keyval_name(pad[1].tb));
		
		fclose(fp);
	}
}

void gtkui_input_config_item(int pnum, int bnum) {
	
}

void gtkui_input_null() {}

int gtkui_input_process_key(GtkWidget *widget, GdkEventKey *event, gpointer userdata) {
	// Process input from GDK events
	
	nesinput_t input;

	input.nescode = input.player = input.pressed = input.turboa = input.turbob = 0;
	
	for (int i = 0; i < NUMGAMEPADS; i++) {
		if (event->keyval == pad[i].u) { input.player = i; input.nescode = Input::Controllers::Pad::UP; }
		else if (event->keyval == pad[i].d) { input.player = i; input.nescode = Input::Controllers::Pad::DOWN; }
		else if (event->keyval == pad[i].l) { input.player = i; input.nescode = Input::Controllers::Pad::LEFT; }
		else if (event->keyval == pad[i].r) { input.player = i; input.nescode = Input::Controllers::Pad::RIGHT; }
		else if (event->keyval == pad[i].select) { input.player = i; input.nescode = Input::Controllers::Pad::SELECT; }
		else if (event->keyval == pad[i].start) { input.player = i; input.nescode = Input::Controllers::Pad::START; }
		else if (event->keyval == pad[i].a) { input.player = i; input.nescode = Input::Controllers::Pad::A; }
		else if (event->keyval == pad[i].b) { input.player = i; input.nescode = Input::Controllers::Pad::B; }
		else if (event->keyval == pad[i].ta) { input.player = i; input.turboa = 1; input.nescode = Input::Controllers::Pad::A; }
		else if (event->keyval == pad[i].tb) { input.player = i; input.turbob = 1; input.nescode = Input::Controllers::Pad::B; }	
	}
	
	switch(event->type) {
		case GDK_KEY_PRESS:
			//printf("Keyval: %x\n", event->keyval);
			//printf("Keyval: %s\n", gdk_keyval_name(event->keyval));
			input.pressed = 1;
			if (event->keyval == ui.qsave1) { nst_state_quicksave(0); }
			else if (event->keyval == ui.qsave2) { nst_state_quicksave(1); }
			else if (event->keyval == ui.qload1) { nst_state_quickload(0); }
			else if (event->keyval == ui.qload2) { nst_state_quickload(1); }
			else if (event->keyval == ui.screenshot) { video_screenshot(NULL); }
			else if (event->keyval == ui.fdsflip) { nst_flip_disk(); }
			else if (event->keyval == ui.fdsswitch) { nst_switch_disk(); }
			else if (event->keyval == ui.insertcoin1) { cNstPads->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_1; }
			else if (event->keyval == ui.insertcoin2) { cNstPads->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_2; }
			else if (event->keyval == ui.reset) { nst_reset(0); }
			else if (event->keyval == ui.ffspeed) { nst_timing_set_ffspeed(); }
			else if (event->keyval == ui.rwstart) { nst_set_rewind(0); }
			else if (event->keyval == ui.rwstop) { nst_set_rewind(1); }
			else if (event->keyval == ui.filter) { video_toggle_filter(); }
			else if (event->keyval == ui.scalefactor) { video_toggle_scalefactor(); }
			else if (event->keyval == gdk_keyval_from_name("space")) { cNstPads->pad[1].mic = 0x04; }
			break;
		case GDK_KEY_RELEASE:
			input.pressed = 0;			
			if (event->keyval == ui.ffspeed) { nst_timing_set_default(); }
			else if (event->keyval == ui.fullscreen) { video_toggle_fullscreen(); }
			else if (event->keyval == gdk_keyval_from_name("space")) { cNstPads->pad[1].mic = 0x00; }
			break;
		default: break;
	}
	
	input_inject(cNstPads, input);
	
	return TRUE;	
}

int gtkui_input_process_mouse(GtkWidget *widget, GdkEventButton *event, gpointer userdata) {
	
	switch(event->type) {
		case GDK_BUTTON_PRESS:
			input_inject_mouse(cNstPads, event->button, 1, (int)event->x, (int)event->y);
			break;
		
		case GDK_BUTTON_RELEASE:
			input_inject_mouse(cNstPads, event->button, 0, (int)event->x, (int)event->y);
			break;
		default: break;
	}
	
	return TRUE;
}
