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
#include <sys/stat.h>

#include "../main.h"
#include "../config.h"
#include "../video.h"

#include "gtkui.h"
#include "gtkui_callbacks.h"
#include "gtkui_config.h"
#include "gtkui_cheats.h"
#include "gtkui_dialogs.h"

GtkWidget *gtkwindow;
GtkWidget *statusbar;
GtkWidget *drawingarea;

char iconpath[512];
char padpath[512];

extern dimensions_t basesize, rendersize;
extern settings_t conf;

void gtkui_init(int argc, char *argv[]) {
	// Initialize the GTK+ GUI
	gtk_init(&argc, &argv);
	#ifndef _APPLE
	gtkui_create();
	#endif
}

void gtkui_state_quickload(GtkWidget *widget, gpointer userdata) {
	// Wrapper function to quickload states
	nst_state_quickload(GPOINTER_TO_INT(userdata));
}

void gtkui_state_quicksave(GtkWidget *widget, gpointer userdata) {
	// Wrapper function to quicksave states
	nst_state_quicksave(GPOINTER_TO_INT(userdata));
}

void gtkui_open_recent(GtkWidget *widget, gpointer userdata) {
	// Open a recently used item
	gchar *uri = gtk_recent_chooser_get_current_uri((GtkRecentChooser*)widget);
	nst_load(g_filename_from_uri(uri, NULL, NULL));
}

void gtkui_create() {
	// Create the GTK+ Window
	
	gtkui_image_paths();
	GdkPixbuf *icon = gdk_pixbuf_new_from_file(iconpath, NULL);
	
	char title[24];
	snprintf(title, sizeof(title), "Nestopia UE %s", VERSION);
	
	gtkwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_icon(GTK_WINDOW(gtkwindow), icon);
	gtk_window_set_title(GTK_WINDOW(gtkwindow), title);
	gtk_window_set_resizable(GTK_WINDOW(gtkwindow), FALSE);
	
	GtkWidget *box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(gtkwindow), box);
	
	// Define the menubar and menus
	GtkWidget *menubar = gtk_menu_bar_new();
	
	// Define the File menu
	GtkWidget *filemenu = gtk_menu_new();
	GtkWidget *file = gtk_menu_item_new_with_mnemonic("_File");
	GtkWidget *open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	GtkWidget *recent = gtk_image_menu_item_new_with_label("Open Recent");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(recent), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU));
	GtkWidget *sep_open = gtk_separator_menu_item_new();
	GtkWidget *stateload = gtk_image_menu_item_new_with_mnemonic("_Load State...");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(stateload), gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU));
	GtkWidget *statesave = gtk_image_menu_item_new_with_mnemonic("_Save State...");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(statesave), gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU));
	
	GtkWidget *quickload = gtk_image_menu_item_new_with_label("Quick Load");
	GtkWidget *qloadmenu = gtk_menu_new();
		GtkWidget *qload0 = gtk_image_menu_item_new_with_label("0");
		GtkWidget *qload1 = gtk_image_menu_item_new_with_label("1");
		GtkWidget *qload2 = gtk_image_menu_item_new_with_label("2");
		GtkWidget *qload3 = gtk_image_menu_item_new_with_label("3");
		GtkWidget *qload4 = gtk_image_menu_item_new_with_label("4");
	
	GtkWidget *quicksave = gtk_image_menu_item_new_with_label("Quick Save");
	GtkWidget *qsavemenu = gtk_menu_new();
		GtkWidget *qsave0 = gtk_image_menu_item_new_with_label("0");
		GtkWidget *qsave1 = gtk_image_menu_item_new_with_label("1");
		GtkWidget *qsave2 = gtk_image_menu_item_new_with_label("2");
		GtkWidget *qsave3 = gtk_image_menu_item_new_with_label("3");
		GtkWidget *qsave4 = gtk_image_menu_item_new_with_label("4");
	
	GtkWidget *sep_state = gtk_separator_menu_item_new();
	GtkWidget *screenshot = gtk_image_menu_item_new_with_mnemonic("S_creenshot...");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(screenshot), gtk_image_new_from_stock(GTK_STOCK_SELECT_COLOR, GTK_ICON_SIZE_MENU));
	GtkWidget *sep_screenshot = gtk_separator_menu_item_new();
	GtkWidget *movieload = gtk_image_menu_item_new_with_label("Load Movie...");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(movieload), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU));
	GtkWidget *moviesave = gtk_image_menu_item_new_with_label("Record Movie...");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(moviesave), gtk_image_new_from_stock(GTK_STOCK_MEDIA_RECORD, GTK_ICON_SIZE_MENU));
	GtkWidget *moviestop = gtk_image_menu_item_new_with_label("Stop Movie");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(moviestop), gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_MENU));
	GtkWidget *sep_movie = gtk_separator_menu_item_new();
	GtkWidget *quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	
	// Set up the recently used items
	GtkWidget *recent_items = gtk_recent_chooser_menu_new();
	GtkRecentFilter *recent_filter = gtk_recent_filter_new();
		gtk_recent_filter_add_pattern(recent_filter, "*.nes");
		gtk_recent_filter_add_pattern(recent_filter, "*.fds");
		gtk_recent_filter_add_pattern(recent_filter, "*.unf");
		gtk_recent_filter_add_pattern(recent_filter, "*.unif");
		gtk_recent_filter_add_pattern(recent_filter, "*.nsf");
		gtk_recent_filter_add_pattern(recent_filter, "*.zip");
		gtk_recent_filter_add_pattern(recent_filter, "*.7z");
		gtk_recent_filter_add_pattern(recent_filter, "*.txz");
		gtk_recent_filter_add_pattern(recent_filter, "*.tar.xz");
		gtk_recent_filter_add_pattern(recent_filter, "*.xz");
		gtk_recent_filter_add_pattern(recent_filter, "*.tgz");
		gtk_recent_filter_add_pattern(recent_filter, "*.tar.gz");
		gtk_recent_filter_add_pattern(recent_filter, "*.gz");
		gtk_recent_filter_add_pattern(recent_filter, "*.tbz");
		gtk_recent_filter_add_pattern(recent_filter, "*.tar.bz2");
		gtk_recent_filter_add_pattern(recent_filter, "*.bz2");
	gtk_recent_chooser_add_filter(GTK_RECENT_CHOOSER(recent_items), recent_filter);
	
	// Populate the File menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), recent);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(recent), recent_items);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep_open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), stateload);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), statesave);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quickload);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(quickload), qloadmenu);
		gtk_menu_shell_append(GTK_MENU_SHELL(qloadmenu), qload0);
		gtk_menu_shell_append(GTK_MENU_SHELL(qloadmenu), qload1);
		gtk_menu_shell_append(GTK_MENU_SHELL(qloadmenu), qload2);
		gtk_menu_shell_append(GTK_MENU_SHELL(qloadmenu), qload3);
		gtk_menu_shell_append(GTK_MENU_SHELL(qloadmenu), qload4);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quicksave);
		gtk_menu_item_set_submenu(GTK_MENU_ITEM(quicksave), qsavemenu);
		gtk_menu_shell_append(GTK_MENU_SHELL(qsavemenu), qsave0);
		gtk_menu_shell_append(GTK_MENU_SHELL(qsavemenu), qsave1);
		gtk_menu_shell_append(GTK_MENU_SHELL(qsavemenu), qsave2);
		gtk_menu_shell_append(GTK_MENU_SHELL(qsavemenu), qsave3);
		gtk_menu_shell_append(GTK_MENU_SHELL(qsavemenu), qsave4);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep_state);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), screenshot);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep_screenshot);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), movieload);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), moviesave);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), moviestop);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep_movie);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	
	// Define the Emulator menu
	GtkWidget *emulatormenu = gtk_menu_new();
	GtkWidget *emu = gtk_menu_item_new_with_mnemonic("_Emulator");
	GtkWidget *cont = gtk_image_menu_item_new_with_mnemonic("C_ontinue");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cont), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU));
	GtkWidget *pause = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PAUSE, NULL);
	GtkWidget *sep_pause = gtk_separator_menu_item_new();
	GtkWidget *resetsoft = gtk_image_menu_item_new_with_mnemonic("_Reset (Soft)");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(resetsoft), gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
	GtkWidget *resethard = gtk_image_menu_item_new_with_mnemonic("Reset (_Hard)");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(resethard), gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
	GtkWidget *sep_reset = gtk_separator_menu_item_new();
	GtkWidget *fullscreen = gtk_image_menu_item_new_from_stock(GTK_STOCK_FULLSCREEN, NULL);
	GtkWidget *sep_fullscreen = gtk_separator_menu_item_new();
	GtkWidget *diskflip = gtk_image_menu_item_new_with_mnemonic("Flip FDS _Disk");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(diskflip), gtk_image_new_from_stock(GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
	GtkWidget *diskswitch = gtk_image_menu_item_new_with_mnemonic("_Switch FDS Disk");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(diskswitch), gtk_image_new_from_stock(GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
	GtkWidget *sep_disk = gtk_separator_menu_item_new();
	GtkWidget *cheats = gtk_image_menu_item_new_with_mnemonic("Ch_eats...");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cheats), gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU));
	GtkWidget *sep_cheats = gtk_separator_menu_item_new();
	GtkWidget *configuration = gtk_image_menu_item_new_with_mnemonic("_Configuration...");
		gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(configuration), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));
	
	// Populate the Emulator menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(emu), emulatormenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cont);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), pause);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep_pause);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), resetsoft);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), resethard);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep_reset);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), fullscreen);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep_fullscreen);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), diskflip);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), diskswitch);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep_disk);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cheats);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep_cheats);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), configuration);
	
	// Define the Help menu
	GtkWidget *helpmenu = gtk_menu_new();
	GtkWidget *help = gtk_menu_item_new_with_mnemonic("_Help");
	GtkWidget *about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);
	
	// Populate the Help menu
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
	
	// Put the menus into the menubar
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), emu);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
	
	// Create the DrawingArea/OpenGL context
	drawingarea = gtk_drawing_area_new();
	//gtk_widget_set_double_buffered(drawingarea, FALSE);
	
	g_object_set_data(G_OBJECT(gtkwindow), "area", drawingarea);
	
	// Set the Drawing Area to be the size of the game output
	gtk_widget_set_size_request(drawingarea, rendersize.w, rendersize.h);
	
	// Create the statusbar
	GtkWidget *statusbar = gtk_statusbar_new();
	
	// Pack the box with the menubar, drawingarea, and statusbar
	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), drawingarea, TRUE, TRUE, 0);
	//gtk_box_pack_start(GTK_BOX(box), statusbar, FALSE, FALSE, 0);
	
	// Make it dark if there's a dark theme
	GtkSettings *gtksettings = gtk_settings_get_default();
	g_object_set(G_OBJECT(gtksettings), "gtk-application-prefer-dark-theme", TRUE, NULL);
	
	// Set up the Drag and Drop target
	GtkTargetEntry target_entry[1];

	target_entry[0].target = (gchar*)"text/uri-list";
	target_entry[0].flags = 0;
	target_entry[0].info = 0;
	
	gtk_drag_dest_set(drawingarea, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP), 
		target_entry, sizeof(target_entry) / sizeof(GtkTargetEntry), (GdkDragAction)(GDK_ACTION_MOVE | GDK_ACTION_COPY));
	
	// Connect the signals
	
	g_signal_connect(G_OBJECT(drawingarea), "drag-data-received",
		G_CALLBACK(gtkui_drag_data), NULL);
	
	g_signal_connect(G_OBJECT(gtkwindow), "delete_event",
		G_CALLBACK(nst_schedule_quit), NULL);
	
	// File menu
	g_signal_connect(G_OBJECT(open), "activate",
		G_CALLBACK(gtkui_file_open), NULL);
	
	g_signal_connect(G_OBJECT(recent_items), "item-activated",
		G_CALLBACK(gtkui_open_recent), NULL);
	
	g_signal_connect(G_OBJECT(stateload), "activate",
		G_CALLBACK(gtkui_state_load), NULL);
	
	g_signal_connect(G_OBJECT(statesave), "activate",
		G_CALLBACK(gtkui_state_save), NULL);
	
	g_signal_connect(G_OBJECT(qload0), "activate",
		G_CALLBACK(gtkui_state_quickload), gpointer(0));
	
	g_signal_connect(G_OBJECT(qload1), "activate",
		G_CALLBACK(gtkui_state_quickload), gpointer(1));
	
	g_signal_connect(G_OBJECT(qload2), "activate",
		G_CALLBACK(gtkui_state_quickload), gpointer(2));
	
	g_signal_connect(G_OBJECT(qload3), "activate",
		G_CALLBACK(gtkui_state_quickload), gpointer(3));
	
	g_signal_connect(G_OBJECT(qload4), "activate",
		G_CALLBACK(gtkui_state_quickload), gpointer(4));
	
	g_signal_connect(G_OBJECT(qsave0), "activate",
		G_CALLBACK(gtkui_state_quicksave), gpointer(0));
	
	g_signal_connect(G_OBJECT(qsave1), "activate",
		G_CALLBACK(gtkui_state_quicksave), gpointer(1));
	
	g_signal_connect(G_OBJECT(qsave2), "activate",
		G_CALLBACK(gtkui_state_quicksave), gpointer(2));
	
	g_signal_connect(G_OBJECT(qsave3), "activate",
		G_CALLBACK(gtkui_state_quicksave), gpointer(3));
	
	g_signal_connect(G_OBJECT(qsave4), "activate",
		G_CALLBACK(gtkui_state_quicksave), gpointer(4));
	
	g_signal_connect(G_OBJECT(screenshot), "activate",
		G_CALLBACK(gtkui_screenshot_save), NULL);
	
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
	
	g_signal_connect(G_OBJECT(fullscreen), "activate",
		G_CALLBACK(video_toggle_fullscreen), NULL);
	
	g_signal_connect(G_OBJECT(diskflip), "activate",
		G_CALLBACK(nst_flip_disk), NULL);
	
	g_signal_connect(G_OBJECT(diskswitch), "activate",
		G_CALLBACK(nst_switch_disk), NULL);
	
	g_signal_connect(G_OBJECT(cheats), "activate",
		G_CALLBACK(gtkui_cheats), NULL);
	
	g_signal_connect(G_OBJECT(configuration), "activate",
		G_CALLBACK(gtkui_config), NULL);
	
	// Help menu
	g_signal_connect(G_OBJECT(about), "activate",
		G_CALLBACK(gtkui_about), NULL);
	
	// Key translation
	g_signal_connect(G_OBJECT(gtkwindow), "key-press-event",
		G_CALLBACK(gtkui_cb_convert_key), NULL);
	
	g_signal_connect(G_OBJECT(gtkwindow), "key-release-event",
		G_CALLBACK(gtkui_cb_convert_key), NULL);
	
	// Mouse translation
	gtk_widget_add_events(GTK_WIDGET(drawingarea), GDK_BUTTON_PRESS_MASK);
	gtk_widget_add_events(GTK_WIDGET(drawingarea), GDK_BUTTON_RELEASE_MASK);
	
	g_signal_connect(G_OBJECT(drawingarea), "button-press-event",
		G_CALLBACK(gtkui_cb_convert_mouse), NULL);
	
	g_signal_connect(G_OBJECT(drawingarea), "button-release-event",
		G_CALLBACK(gtkui_cb_convert_mouse), NULL);
	
	gtk_widget_show_all(gtkwindow);
}

void gtkui_resize() {
	// Resize the GTK+ window
	if (gtkwindow) {
		gtk_widget_set_size_request(drawingarea, rendersize.w, rendersize.h);
	}
}

void gtkui_set_title(const char *title) {
	#ifndef _APPLE
	gtk_window_set_title(GTK_WINDOW(gtkwindow), title);
	#endif
}

GtkWidget *gtkui_about() {
	// Pull up the About dialog
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(iconpath, 192, 192, NULL);
	GtkWidget *aboutdialog = gtk_about_dialog_new();
	gtk_window_set_transient_for(GTK_WINDOW(aboutdialog), GTK_WINDOW(gtkwindow));
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutdialog), pixbuf);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutdialog), "Nestopia UE");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), VERSION);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutdialog), "Cycle-Accurate Nintendo Entertainment System Emulator");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutdialog), "http://0ldsk00l.ca/nestopia/");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutdialog), "(c) 2012-2016, R. Danbrook\n(c) 2007-2008, R. Belmont\n(c) 2003-2008, Martin Freij\n\nIcon based on art from Trollekop");
	gtk_dialog_run(GTK_DIALOG(aboutdialog));
	gtk_widget_destroy(aboutdialog);
	
	return aboutdialog;
}

void gtkui_image_paths() {
	// Set paths to SVG icons/images
	snprintf(iconpath, sizeof(iconpath), "%s/icons/nestopia.svg", DATADIR);
	snprintf(padpath, sizeof(padpath), "%s/icons/nespad.svg", DATADIR);
	
	// Load the SVG from local source dir if make install hasn't been done
	struct stat svgstat;
	if (stat(iconpath, &svgstat) == -1) {
		snprintf(iconpath, sizeof(iconpath), "source/unix/icons/nestopia.svg");
	}
	if (stat(padpath, &svgstat) == -1) {
		snprintf(padpath, sizeof(padpath), "source/unix/icons/nespad.svg");
	}
}

void gtkui_message(const char* message) {    
	GtkWidget *messagewindow = gtk_message_dialog_new(
				GTK_WINDOW(gtkwindow),
				GTK_DIALOG_DESTROY_WITH_PARENT,
				GTK_MESSAGE_INFO,
				GTK_BUTTONS_OK,
				message);
	gtk_dialog_run(GTK_DIALOG(messagewindow));
	gtk_widget_destroy(messagewindow);
}

void gtkui_cursor_set_crosshair() {
	// Set the cursor to a crosshair
	GdkCursor *cursor;
	cursor = gdk_cursor_new(GDK_CROSSHAIR);
	
	GdkWindow *gdkwindow = gtk_widget_get_window(GTK_WIDGET(drawingarea));
	
	gdk_window_set_cursor(gdkwindow, cursor);
	gdk_flush();
	gdk_cursor_unref(cursor);
}

void gtkui_cursor_set_default() {
	// Set the cursor to the default
	GdkCursor *cursor;
	cursor = gdk_cursor_new(GDK_LEFT_PTR);
	
	GdkWindow *gdkwindow = gtk_widget_get_window(GTK_WIDGET(drawingarea));
	
	gdk_window_set_cursor(gdkwindow, cursor);
	gdk_flush();
	gdk_cursor_unref(cursor);
}
