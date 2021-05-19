/*
 * Nestopia UE
 *
 * Copyright (C) 2012-2021 R. Danbrook
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

#include <cstdio>
#include <sys/stat.h>

#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Bar.H>
#include <FL/Fl_Native_File_Chooser.H>
#include <FL/Fl_PNG_Image.H>
#include <FL/Fl_Gl_Window.H>
#include <FL/gl.h>

#include <SDL.h>

#include "nstcommon.h"
#include "cli.h"
#include "config.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "cheats.h"

#include "fltkui.h"
#include "fltkui_archive.h"
#include "fltkui_cheats.h"
#include "fltkui_config.h"

#define MBARHEIGHT 24

static NstWindow *nstwin;
static Fl_Menu_Bar *menubar;
static NstGlArea *glarea;
static NstChtWindow *chtwin;
static NstConfWindow *confwin;

extern int loaded;

Fl_Color NstGreen = 0x255f6500;
Fl_Color NstPurple = 0x5f578700;
Fl_Color NstRed = 0xb51e2c00;
Fl_Color NstBlueGrey = 0x383c4a00;
Fl_Color NstLightGrey = 0xd3dae300;

extern Input::Controllers *cNstPads;
extern nstpaths_t nstpaths;

extern bool (*nst_archive_select)(const char*, char*, size_t);

static void fltkui_cheats(Fl_Widget* w, void* userdata) {
	if (!loaded) { return; }
	chtwin->refresh();
	chtwin->show();
}

static void fltkui_config(Fl_Widget* w, void* userdata) {
	confwin->show();
}

static void fltkui_rom_open(Fl_Widget* w, void* userdata) {
	// Create native chooser
	Fl_Native_File_Chooser fc;
	fc.title("Select a ROM");
	fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fc.filter("NES Games\t*.{nes,unf,fds,zip,7z,gz,bz2,xz}");

	// Show file chooser
	switch (fc.show()) {
		case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
		case 1: break; // Cancel
		default:
			if (fc.filename()) {
				loaded = nst_load(fc.filename());
				nstwin->label(nstpaths.gamename);
				if (loaded) { nst_play(); }
			}
			break;
	}
}

static void fltkui_movie_load(Fl_Widget* w, void* userdata) {
	// Create native chooser
	if (!loaded) { return; }

	Fl_Native_File_Chooser fc;
	fc.title("Select a Movie");
	fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fc.directory((const char*)nstpaths.nstdir);
	fc.filter("Nestopia Movies\t*.nsv");

	// Show file chooser
	switch (fc.show()) {
		case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
		case 1: break; // Cancel
		default:
			if (fc.filename()) {
				nst_movie_load(fc.filename());
			}
			break;
	}
}

static void fltkui_movie_save(Fl_Widget* w, void* userdata) {
	// Create native chooser
	if (!loaded) { return; }

	Fl_Native_File_Chooser fc;
	fc.title("Save Movie");
	fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	fc.directory((const char*)nstpaths.nstdir);
	fc.filter("Nestopia Moviess\t*.nsv");
	fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

	// Show file chooser
	if (fc.show()) { return; }

	nst_movie_save(fc.filename());
}

static void fltkui_movie_stop(Fl_Widget* w, void* userdata) {
	nst_movie_stop();
}

static void fltkui_state_load(Fl_Widget* w, void* userdata) {
	// Create native chooser
	if (!loaded) { return; }

	Fl_Native_File_Chooser fc;
	fc.title("Load State");
	fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fc.directory((const char*)nstpaths.statepath);
	fc.filter("Nestopia States\t*.nst");

	// Show file chooser
	switch (fc.show()) {
		case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
		case 1: break; // Cancel
		default:
			if (fc.filename()) {
				nst_state_load(fc.filename());
			}
			break;
	}
}

static void fltkui_state_save(Fl_Widget* w, void* userdata) {
	// Create native chooser
	if (!loaded) { return; }

	Fl_Native_File_Chooser fc;
	fc.title("Save State");
	fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	fc.directory((const char*)nstpaths.statepath);
	fc.filter("Nestopia States\t*.nst");
	fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

	// Show file chooser
	if (fc.show()) { return; }

	nst_state_save(fc.filename());
}

static void fltkui_screenshot(Fl_Widget* w, void* userdata) {
	// Create native chooser
	if (!loaded) { return; }

	Fl_Native_File_Chooser fc;
	fc.title("Save Screenshot");
	fc.type(Fl_Native_File_Chooser::BROWSE_SAVE_FILE);
	fc.directory((const char*)nstpaths.nstdir);
	fc.filter("PNG Screenshots\t*.png");
	fc.options(Fl_Native_File_Chooser::SAVEAS_CONFIRM | Fl_Native_File_Chooser::USE_FILTER_EXT);

	// Show file chooser
	if (fc.show()) { return; }

	video_screenshot(fc.filename());
}

static void fltkui_palette_open(Fl_Widget* w, void* userdata) {
	// Create native chooser
	Fl_Native_File_Chooser fc;
	fc.title("Select a Palette");
	fc.type(Fl_Native_File_Chooser::BROWSE_FILE);
	fc.filter("NES Palettes\t*.pal");

	// Show file chooser
	switch (fc.show()) {
		case -1: fprintf(stderr, "Error: %s\n", fc.errmsg()); break;
		case 1: break; // Cancel
		default:
			if (fc.filename()) {
				nst_palette_load(fc.filename());
				nst_palette_save();
				conf.video_palette_mode = 2;
				video_init();
			}
			break;
	}
}

static void fltkui_state_qload(Fl_Widget* w, void* userdata) {
	nst_state_quickload(atoi((const char*)userdata));
}

static void fltkui_state_qsave(Fl_Widget* w, void* userdata) {
	nst_state_quicksave(atoi((const char*)userdata));
}

static void fltkui_pause(Fl_Widget* w, void* userdata) {
	if (nst_playing()) {
		nst_pause();
	}
	else {
		nst_play();
	}
}

static void fltkui_reset(Fl_Widget* w, void* userdata) {
	nst_reset(atoi((const char*)userdata));
}

void fltkui_resize() {
	video_set_dimensions();
	dimensions_t rendersize = nst_video_get_dimensions_render();
	nstwin->size(rendersize.w, rendersize.h + MBARHEIGHT);
	menubar->resize(0, 0, nstwin->w(), MBARHEIGHT);
	glarea->resize(0, 24, rendersize.w, rendersize.h);
	nst_video_set_dimensions_screen(rendersize);
	video_init();
}

void fltkui_fullscreen(Fl_Widget* w, void* userdata) {
	if (!nst_playing()) { return; }

	conf.video_fullscreen ^= 1;

	if (conf.video_fullscreen) {
		int x, y, w, h;
		Fl::screen_xywh(x, y, w, h);
		menubar->hide();
		nstwin->fullscreen();
		dimensions_t scrdim = {w, h};
		nstwin->resize(0, 0, scrdim.w, scrdim.h);
		glarea->resize(0, 0, scrdim.w, scrdim.h);
		nst_video_set_dimensions_screen(scrdim);
		video_init();
	}
	else {
		video_set_dimensions();
		dimensions_t rendersize = nst_video_get_dimensions_render();
		nstwin->fullscreen_off();
		nstwin->size(rendersize.w, rendersize.h + MBARHEIGHT);
		menubar->show();
		menubar->resize(0, 0, nstwin->w(), MBARHEIGHT);
		glarea->resize(0, 24, rendersize.w, rendersize.h);
		nst_video_set_dimensions_screen(rendersize);
		video_init();
	}
}

static void fltkui_fds_flip(Fl_Widget* w, void* userdata) {
	nst_fds_flip();
}

static void fltkui_fds_switch(Fl_Widget* w, void* userdata) {
	nst_fds_switch();
}

static void fltkui_about_close(Fl_Widget* w, void* userdata) {
	Fl_Window *about = (Fl_Window*)userdata;
	about->hide();
}

static void fltkui_about(Fl_Widget* w, void* userdata) {
	Fl_Window about(460, 440);
	Fl_Box iconbox(166, 16, 128, 128);

	Fl_Box text0(0, 144, 460, 24, "Nestopia UE");
	text0.labelfont(FL_BOLD);

	Fl_Box text1(0, 166, 460, 24, "1.51.0");

	Fl_Box text2(0, 208, 460, 24, "Cycle-Accurate Nintendo Entertainment System Emulator");

	Fl_Box text3(0, 256, 460, 24, "FLTK Frontend\n(c) 2012-2021, R. Danbrook\n(c) 2007-2008, R. Belmont");
	text3.labelsize(10);

	Fl_Box text4(0, 320, 460, 24, "Nestopia Emulator\n(c) 2020-2021, Rupert Carmichael\n(c) 2012-2020, Nestopia UE Contributors\n(c) 2003-2008, Martin Freij");
	text4.labelsize(10);

	Fl_Box text5(0, 360, 460, 24, "Icon based on drawing by Trollekop");
	text5.labelsize(10);

	// Set up the icon
	char iconpath[512];
	snprintf(iconpath, sizeof(iconpath), "%s/icons/hicolor/128x128/apps/nestopia.png", DATAROOTDIR);
	// Load the SVG from local source dir if make install hasn't been done
	struct stat svgstat;
	if (stat(iconpath, &svgstat) == -1) {
		snprintf(iconpath, sizeof(iconpath), "icons/128/nestopia.png");
	}

	Fl_PNG_Image nsticon(iconpath);
	iconbox.image(nsticon);

	Fl_Button close(360, 400, 80, 24, "&Close");
	close.callback(fltkui_about_close, (void*)&about);

	about.set_modal();
	about.show();
	while (about.shown()) { Fl::wait(); }
}

static void quit_cb(Fl_Widget* w, void* userdata) {
	nstwin->hide();
}

// this is used to stop Esc from exiting the program:
int handle(int e) {
	return (e == FL_SHORTCUT); // eat all keystrokes
}

int NstWindow::handle(int e) {
	switch (e) { case FL_KEYDOWN: case FL_KEYUP: fltkui_input_process_key(e); break; }
	return Fl_Double_Window::handle(e);
}

int NstGlArea::handle(int e) {
	if (nst_input_zapper_present()) {
		switch (e) {
			case FL_ENTER:
				cursor(conf.misc_disable_cursor_special ? FL_CURSOR_NONE : FL_CURSOR_CROSS);
				break;
			case FL_LEAVE:
				cursor(FL_CURSOR_DEFAULT);
				break;
			case FL_PUSH:
				nst_input_inject_mouse(cNstPads, Fl::event_button(), 1, Fl::event_x(), Fl::event_y());
				break;
			case FL_RELEASE:
				nst_input_inject_mouse(cNstPads, Fl::event_button(), 0, Fl::event_x(), Fl::event_y());
				break;
		}
	}
	else if (e == FL_ENTER && conf.misc_disable_cursor) { cursor(FL_CURSOR_NONE); }
	else if (e == FL_LEAVE) { cursor(FL_CURSOR_DEFAULT); }

	return Fl_Gl_Window::handle(e);
}

static Fl_Menu_Item menutable[] = {
	{"&File", 0, 0, 0, FL_SUBMENU},
		{"&Open", FL_ALT + 'o', fltkui_rom_open, 0, FL_MENU_DIVIDER},
		{"Load State...", 0, fltkui_state_load, 0, 0},
		{"Save State...", 0, fltkui_state_save, 0, FL_MENU_DIVIDER},
		{"Quick Load", 0, 0, 0, FL_SUBMENU},
			{"Slot 0", 0, fltkui_state_qload, (void*)"0", 0},
			{"Slot 1", 0, fltkui_state_qload, (void*)"1", 0},
			{"Slot 2", 0, fltkui_state_qload, (void*)"2", 0},
			{"Slot 3", 0, fltkui_state_qload, (void*)"3", 0},
			{"Slot 4", 0, fltkui_state_qload, (void*)"4", 0},
			{0},
		{"Quick Save", 0, 0, 0, FL_SUBMENU|FL_MENU_DIVIDER},
			{"Slot 0", 0, fltkui_state_qsave, (void*)"0", 0},
			{"Slot 1", 0, fltkui_state_qsave, (void*)"1", 0},
			{"Slot 2", 0, fltkui_state_qsave, (void*)"2", 0},
			{"Slot 3", 0, fltkui_state_qsave, (void*)"3", 0},
			{"Slot 4", 0, fltkui_state_qsave, (void*)"4", 0},
			{0},
		{"Open Palette...", 0, fltkui_palette_open, 0, FL_MENU_DIVIDER},
		{"Screenshot...", 0, fltkui_screenshot, 0, FL_MENU_DIVIDER},
		{"Load Movie...", 0, fltkui_movie_load, 0, 0},
		{"Record Movie...", 0, fltkui_movie_save, 0, 0},
		{"Stop Movie", 0, fltkui_movie_stop, 0, FL_MENU_DIVIDER},
		{"&Quit", FL_ALT + 'q', quit_cb},
		{0}, // End File
	{"&Emulator", 0, 0, 0, FL_SUBMENU},
		{"Pause/Play", 0, fltkui_pause, 0, FL_MENU_DIVIDER},
		{"Reset (Soft)", 0, fltkui_reset, (void*)"0", 0},
		{"Reset (Hard)", 0, fltkui_reset, (void*)"1", FL_MENU_DIVIDER},
		{"Fullscreen", 0, fltkui_fullscreen, 0, FL_MENU_DIVIDER},
		{"Flip Disk", 0, fltkui_fds_flip, 0, 0},
		{"Switch Disk", 0, fltkui_fds_switch, 0, FL_MENU_DIVIDER},
		{"Cheats...", 0, fltkui_cheats, 0, FL_MENU_DIVIDER},
		{"Configuration...", 0, fltkui_config, 0},
		{0}, // End Emulator
	{"&Help", 0, 0, 0, FL_SUBMENU},
		{"About", 0, fltkui_about, 0, 0},
		{0}, // End Help
	{0} // End Menu
};

void makenstwin(const char *name) {
	video_set_dimensions();
	dimensions_t rendersize = nst_video_get_dimensions_render();

	Fl::add_handler(handle);

	// Cheats Window
	chtwin = new NstChtWindow(660, 500, "Cheat Manager");
	chtwin->populate();

	// Configuration Window
	confwin = new NstConfWindow(400, 400, "Configuration");
	confwin->populate();

	// Main Window
	nstwin = new NstWindow(rendersize.w, rendersize.h + MBARHEIGHT, name);
	nstwin->color(FL_BLACK);
	nstwin->xclass("nestopia");

	// Menu Bar
	menubar = new Fl_Menu_Bar(0, 0, nstwin->w(), MBARHEIGHT);
	menubar->box(FL_FLAT_BOX);
	menubar->menu(menutable);

	glarea = new NstGlArea(0, MBARHEIGHT, nstwin->w(), nstwin->h() - MBARHEIGHT);
	glarea->color(FL_BLACK);

	nstwin->end();
}

int main(int argc, char *argv[]) {
	// Set up directories
	nst_set_dirs();

	// Set default config options
	config_set_default();

	// Read the config file and override defaults
	config_file_read(nstpaths.nstconfdir);

	// Handle command line arguments
	cli_handle_command(argc, argv);

	// Set the video dimensions
	video_set_dimensions();

	// Set up callbacks
	nst_set_callbacks();

	// Initialize SDL Audio and Joystick
	if (SDL_Init(SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}

	// Set archive handler function pointer
	nst_archive_select = &fltkui_archive_select;

	// Detect and set up Joysticks
	nstsdl_input_joysticks_detect();
	nstsdl_input_conf_defaults();
	nstsdl_input_conf_read();

	// Initialize and load FDS BIOS and NstDatabase.xml
	nst_fds_bios_load();
	nst_db_load();

	makenstwin(argv[0]);
	nstwin->label("Nestopia UE");
	nstwin->show();
	menubar->show();
	glarea->make_current();
	glarea->show();

	Fl::check();

	// Load a rom from the command line
	if (argc > 1 && argv[argc - 1][0] != '-') {
		int loaded = nst_load(argv[argc - 1]);
		if (loaded) { nst_play(); }
		else { exit(1); }
		nstwin->label(nstpaths.gamename);
	}
	else if (conf.video_fullscreen) {
		conf.video_fullscreen = 0;
	}

	if (conf.video_fullscreen) {
		conf.video_fullscreen = 0;
		fltkui_fullscreen(NULL, NULL);
	}

	video_init();

	while (true) {
		Fl::check();
		if (!nstwin->shown()) {
			break;
		}
		else if (!nstwin->shown() && (confwin->shown() || chtwin->shown())) {
			break;
		}
		SDL_Event event;
		while (SDL_PollEvent(&event)) {
			switch (event.type) {
				case SDL_JOYHATMOTION:
				case SDL_JOYAXISMOTION:
				case SDL_JOYBUTTONDOWN:
				case SDL_JOYBUTTONUP:
					nstsdl_input_process(cNstPads, event);
					break;
				default: break;
			}
		}

		nst_emuloop();
		glarea->redraw();
	}

	// Remove the cartridge and shut down the NES
	nst_unload();

	// Unload the FDS BIOS, NstDatabase.xml, and the custom palette
	nst_db_unload();
	nst_fds_bios_unload();
	nst_palette_unload();

	// Deinitialize audio
	audio_deinit();

	// Deinitialize joysticks
	nstsdl_input_joysticks_close();

	// Write the input config file
	nstsdl_input_conf_write();

	// Write the config file
	config_file_write(nstpaths.nstconfdir);

	return 0;
}
