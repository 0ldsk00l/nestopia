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

#include "../main.h"
#include "../config.h"
#include "../video.h"

#include "gtkui.h"
#include "gtkui_callbacks.h"
#include "gtkui_config.h"
#include "gtkui_cheats.h"
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
	
	// Define the File menu
	GtkWidget *filemenu = gtk_menu_new();
	GtkWidget *file = gtk_menu_item_new_with_label("File");
	GtkWidget *open = gtk_menu_item_new_with_label("Open...");
	GtkWidget *statesave = gtk_menu_item_new_with_label("Save State...");
	GtkWidget *stateload = gtk_menu_item_new_with_label("Load State...");
	GtkWidget *moviesave = gtk_menu_item_new_with_label("Movie Save");
	GtkWidget *movieload = gtk_menu_item_new_with_label("Movie Load");
	GtkWidget *moviestop = gtk_menu_item_new_with_label("Movie Stop");
	GtkWidget *quit = gtk_menu_item_new_with_label("Quit");
	
	// Populate the File menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), statesave);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), stateload);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), moviesave);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), movieload);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), moviestop);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	
	// Define the Emulator menu
	GtkWidget *emulatormenu = gtk_menu_new();
	GtkWidget *emu = gtk_menu_item_new_with_label("Emulator");
	GtkWidget *cont = gtk_menu_item_new_with_label("Continue");
	GtkWidget *pause = gtk_menu_item_new_with_label("Pause");
	GtkWidget *resetsoft = gtk_menu_item_new_with_label("Reset (Soft)");
	GtkWidget *resethard = gtk_menu_item_new_with_label("Reset (Hard)");
	GtkWidget *cheats = gtk_menu_item_new_with_label("Cheats");
	GtkWidget *configuration = gtk_menu_item_new_with_label("Configuration");
	
	// Populate the Emulator menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(emu), emulatormenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cont);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), pause);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), resetsoft);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), resethard);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cheats);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), configuration);
	
	// Define the Help menu
	GtkWidget *helpmenu = gtk_menu_new();
	GtkWidget *help = gtk_menu_item_new_with_label("Help");
	GtkWidget *about = gtk_menu_item_new_with_label("About");
	
	// Populate the Help menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
	
	// Put the menus into the menubar
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), emu);
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
	
	g_signal_connect(G_OBJECT(gtkwindow), "delete_event",
		G_CALLBACK(nst_schedule_quit), NULL);
	
	// File menu
	g_signal_connect(G_OBJECT(open), "activate",
		G_CALLBACK(gtkui_file_open), NULL);
	
	g_signal_connect(G_OBJECT(statesave), "activate",
		G_CALLBACK(gtkui_state_save), NULL);
	
	g_signal_connect(G_OBJECT(stateload), "activate",
		G_CALLBACK(gtkui_state_load), NULL);
	
	g_signal_connect(G_OBJECT(moviesave), "activate",
		G_CALLBACK(gtkui_movie_save), NULL);
	
	g_signal_connect(G_OBJECT(movieload), "activate",
		G_CALLBACK(gtkui_movie_load), NULL);
	
	g_signal_connect(G_OBJECT(moviestop), "activate",
		G_CALLBACK(gtkui_movie_stop), NULL);
	
	g_signal_connect(G_OBJECT(quit), "activate",
		G_CALLBACK(nst_schedule_quit), NULL);
	
	// Emulator menu
	g_signal_connect(G_OBJECT(cont), "activate",
		G_CALLBACK(nst_play), NULL);
	
	g_signal_connect(G_OBJECT(pause), "activate",
		G_CALLBACK(nst_pause), NULL);
	
	g_signal_connect(G_OBJECT(resetsoft), "activate",
		G_CALLBACK(gtkui_cb_reset), gpointer(0));
	
	g_signal_connect(G_OBJECT(resethard), "activate",
		G_CALLBACK(gtkui_cb_reset), gpointer(1));
	
	g_signal_connect(G_OBJECT(cheats), "activate",
		G_CALLBACK(gtkui_cheats), NULL);
	
	g_signal_connect(G_OBJECT(configuration), "activate",
		G_CALLBACK(gtkui_config), NULL);
	
	// Help menu
	g_signal_connect(G_OBJECT(about), "activate",
		G_CALLBACK(gtkui_about), NULL);
	
	// Key translation
	g_signal_connect(G_OBJECT(gtkwindow), "key_press_event",
		G_CALLBACK(gtkui_cb_convert_key), NULL);
	
	g_signal_connect(G_OBJECT(gtkwindow), "key_release_event",
		G_CALLBACK(gtkui_cb_convert_key), NULL);
	
	gtk_widget_show_all(gtkwindow);
	
	gtk_widget_override_background_color(drawingarea, GTK_STATE_FLAG_NORMAL, &bg);
}

void gtkui_toggle_fullscreen() {
	// Toggle fullscreen
	if (conf.video_fullscreen) {
		video_create();
		video_init();
	}
	else {
		video_destroy();
		gtk_opengl_current(drawingarea, context);
		video_init();
		gtkui_resize();
	}
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
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutdialog), pixbuf);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutdialog), "Nestopia UE");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), VERSION);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutdialog), "Cycle-Accurate Nintendo Entertainment System Emulator");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutdialog), "http://0ldsk00l.ca/nestopia/");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutdialog), "(c) 2012-2014, R. Danbrook\n(c) 2007-2008, R. Belmont\n(c) 2003-2008, Martin Freij\n\nIcon based on art from Trollekop");
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

void gtkui_swapbuffers() {
	gtk_opengl_swap(drawingarea);
}
