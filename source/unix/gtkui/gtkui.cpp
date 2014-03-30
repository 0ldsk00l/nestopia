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
#include <sys/stat.h>

#include <SDL.h>

#include "../main.h"
#include "../config.h"
#include "../video.h"

#include "gtkui.h"
#include "gtkui_dialogs.h"
#include "gtk_opengl.h"

GLXContext context;

int attributes[] = {
	GLX_RGBA,
	GLX_RED_SIZE, 1,
	GLX_GREEN_SIZE, 1,
	GLX_BLUE_SIZE, 1,
	GLX_DOUBLEBUFFER, True,
	GLX_DEPTH_SIZE, 12,
	None
};

GtkWidget *gtkwindow;
GtkWidget *statusbar;
GtkWidget *drawingarea;

GdkRGBA bg = {0, 0, 0, 0};

extern dimensions_t basesize, rendersize;
extern settings_t conf;
extern GLuint screenTexID;

void gtkui_init(int argc, char *argv[]) {
	// Initialize the GTK+ GUI
	gtk_init(&argc, &argv);
	
	gtkui_create();
}

void gtkui_create() {
	// Create the GTK+ Window
		
	gtkwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(gtkwindow), "Nestopia");
	gtk_window_set_resizable(GTK_WINDOW(gtkwindow), FALSE);
	
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(gtkwindow), box);
	
	// Define the menubar and menus
	GtkWidget *menubar = gtk_menu_bar_new();
	
	GtkWidget *filemenu = gtk_menu_new();
	GtkWidget *file = gtk_menu_item_new_with_label("File");
	GtkWidget *open = gtk_menu_item_new_with_label("Open...");
	GtkWidget *quit = gtk_menu_item_new_with_label("Quit");
	
	GtkWidget *helpmenu = gtk_menu_new();
	GtkWidget *help = gtk_menu_item_new_with_label("Help");
	GtkWidget *about = gtk_menu_item_new_with_label("About");
	
	// Populate the File menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	
	// Populate the Help menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
	
	// Put the menus into the menubar
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
	
	// Create the DrawingArea/OpenGL context
	context = NULL;
	drawingarea = gtk_drawing_area_new();
	context = gtk_opengl_create(drawingarea, attributes, context, TRUE);
	
	g_object_set_data(G_OBJECT(gtkwindow), "area", drawingarea);
	g_object_set_data(G_OBJECT(gtkwindow), "context", context);
	
	// Set the Drawing Area to be the size of the game output
	gtk_widget_set_size_request(drawingarea, rendersize.w, rendersize.h);
	
	// Create the statusbar
	GtkWidget *statusbar = gtk_statusbar_new();
	
	// Pack the box with the menubar, drawingarea, and statusbar
	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), drawingarea, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(box), statusbar, FALSE, FALSE, 0);
	
	// Make it dark if there's a dark theme
	GtkSettings *gtksettings = gtk_settings_get_default();
	g_object_set(G_OBJECT(gtksettings), "gtk-application-prefer-dark-theme", TRUE, NULL);
	
	// Connect the signals
	g_signal_connect(drawingarea, "realize",
		G_CALLBACK(area_start), gtkwindow);
	
	g_signal_connect(G_OBJECT(gtkwindow), "destroy",
		G_CALLBACK(nst_schedule_quit), NULL);
	
	// File menu
	g_signal_connect(G_OBJECT(quit), "activate",
		G_CALLBACK(nst_schedule_quit), NULL);
	
	g_signal_connect(G_OBJECT(open), "activate",
		G_CALLBACK(gtkui_file_open), NULL);
	
	// Help menu
	g_signal_connect(G_OBJECT(about), "activate",
		G_CALLBACK(gtkui_about), NULL);
	
	// Key translation
	g_signal_connect(G_OBJECT(gtkwindow), "key_press_event",
		G_CALLBACK(convert_keypress), NULL);
	
	g_signal_connect(G_OBJECT(gtkwindow), "key_release_event",
		G_CALLBACK(convert_keypress), NULL);
	
	gtk_widget_show_all(gtkwindow);
	
	gtk_widget_override_background_color(drawingarea, GTK_STATE_FLAG_NORMAL, &bg);
}

void gtkui_resize() {
	// Resize the GTK+ window
	gtk_widget_set_size_request(drawingarea, rendersize.w, rendersize.h);
	gtk_widget_override_background_color(drawingarea, GTK_STATE_FLAG_NORMAL, &bg);
}

GtkWidget *gtkui_about() {

	char svgpath[1024];
	snprintf(svgpath, sizeof(svgpath), "%s/icons/nestopia.svg", DATADIR);
	
	// Load the SVG from local source dir if make install hasn't been done
	struct stat svgstat;
	if (stat(svgpath, &svgstat) == -1) {
		snprintf(svgpath, sizeof(svgpath), "source/unix/icons/nestopia.svg");
	}
	
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(svgpath, 192, 192, NULL);
	
	GtkWidget *aboutdialog = gtk_about_dialog_new();
	
	//GdkPixbuf *app_icon = get_icon();
	//gtk_window_set_icon(GTK_WINDOW(aboutdialog), app_icon);
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutdialog), pixbuf);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutdialog), "Nestopia UE");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), VERSION);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutdialog), "Cycle-Accurate Nintendo Entertainment System Emulator");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutdialog), "http://0ldsk00l.ca/nestopia/");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutdialog), "(c) 2012-2014, R. Danbrook\n(c) 2007-2008, R. Belmont\n(c) 2003-2008, Martin Freij\n\nIcon based on art from Trollekop");
	//g_object_unref(pixbuf), pixbuf = NULL;
	gtk_dialog_run(GTK_DIALOG(aboutdialog));
	gtk_widget_destroy(aboutdialog);
	
	return aboutdialog;
}

int area_start(GtkWidget *widget, void *data) {
	
	GtkWidget *window = (GtkWidget *) data;
	GtkWidget *area = (GtkWidget *) g_object_get_data (G_OBJECT (window), "area");
	GLXContext context = (GLXContext) g_object_get_data (G_OBJECT (window), "context");

	if (gtk_opengl_current(area, context) == TRUE) {
		glEnable (GL_DEPTH_TEST);
		glDepthFunc (GL_LEQUAL);
		glEnable (GL_CULL_FACE);
		glCullFace (GL_BACK);
		glDisable (GL_DITHER);
		glShadeModel (GL_SMOOTH);
	}
}

unsigned int translate_gdk_sdl(int gdk_keyval) {
	
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

int convert_keypress(GtkWidget *grab, GdkEventKey *event, gpointer user_data) {

	SDL_Event sdlevent;
	SDL_Keycode sdlkeycode;
	int keystate;
	
	switch (event->type)
	{
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
	
	sdlkeycode = (SDL_Keycode)translate_gdk_sdl(event->keyval);
	sdlevent.key.keysym.sym = sdlkeycode;
	sdlevent.key.keysym.scancode = SDL_GetScancodeFromKey(sdlevent.key.keysym.sym);
		
	if (sdlkeycode != 0) {
		SDL_PushEvent(&sdlevent);
		
		const Uint8 *statebuffer = SDL_GetKeyboardState(NULL);
		Uint8 *state = (Uint8*)statebuffer;
		state[SDL_GetScancodeFromKey(sdlkeycode)] = keystate;
	}
	// Allow GTK+ to process this key.
	return FALSE;
}

void gtkui_swapbuffers() {
	gtk_opengl_swap(drawingarea);
}
