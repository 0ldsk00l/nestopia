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

#include "../main.h"
#include "../video.h"

#include "gtkui.h"
#include "gtkui_cheats.h"

extern nstpaths_t nstpaths;
extern GtkWidget *gtkwindow;

void gtkui_file_open() {
	// Open a file using a GTK+ dialog
	GtkWidget *dialog = gtk_file_chooser_dialog_new(
				"Select a ROM",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN,
				GTK_RESPONSE_ACCEPT,
				NULL);
	
	GtkFileFilter *filter = gtk_file_filter_new();
	
	gtk_file_filter_set_name(filter, "NES ROMs and Archives");
	gtk_file_filter_add_pattern(filter, "*.nes");
	gtk_file_filter_add_pattern(filter, "*.fds");
	gtk_file_filter_add_pattern(filter, "*.unf");
	gtk_file_filter_add_pattern(filter, "*.unif");
	gtk_file_filter_add_pattern(filter, "*.nsf");
	gtk_file_filter_add_pattern(filter, "*.zip");
	gtk_file_filter_add_pattern(filter, "*.7z");
	gtk_file_filter_add_pattern(filter, "*.txz");
	gtk_file_filter_add_pattern(filter, "*.tar.xz");
	gtk_file_filter_add_pattern(filter, "*.xz");
	gtk_file_filter_add_pattern(filter, "*.tgz");
	gtk_file_filter_add_pattern(filter, "*.tar.gz");
	gtk_file_filter_add_pattern(filter, "*.gz");
	gtk_file_filter_add_pattern(filter, "*.tbz");
	gtk_file_filter_add_pattern(filter, "*.tar.bz2");
	gtk_file_filter_add_pattern(filter, "*.bz2");
	
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtk_widget_destroy(dialog);
		nst_load(filename);
		g_free(filename);
	}
	else { gtk_widget_destroy(dialog); }
}

void gtkui_state_save() {
	// Save a state from the GUI
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Save State (.nst)",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
	
	char statepath[512];
	snprintf(statepath, sizeof(statepath), "%s.nst", nstpaths.statepath);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), statepath);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), nstpaths.statepath);
	
	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		nst_state_save(filename);
		g_free(filename);
	}
	
	gtk_widget_destroy(dialog);
}

void gtkui_state_load() {
	// Load a state from the GUI
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Load State (.nst)",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Nestopia Save States");
	gtk_file_filter_add_pattern(filter, "*.nst");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), nstpaths.statepath);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		nst_state_load(filename);
		g_free(filename);
	}
	
	gtk_widget_destroy(dialog);
}

void gtkui_screenshot_save() {
	// Save a screenshot from the GUI
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Save screenshot (.png)",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
	
	char sshotpath[512];
	char sshotfile[768];
	snprintf(sshotpath, sizeof(sshotpath), "%sscreenshots/", nstpaths.nstdir);
	snprintf(sshotfile, sizeof(sshotfile), "%s%s.png", sshotpath, nstpaths.gamename);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), sshotfile);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), sshotpath);
	
	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		video_screenshot(filename);
		g_free(filename);
	}
	
	gtk_widget_destroy(dialog);
}

void gtkui_movie_save() {
	// Save a movie from the GUI
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Save movie (.nsv)",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_SAVE,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
				NULL);
	
	char moviepath[512];
	snprintf(moviepath, sizeof(moviepath), "%s%s.nsv", nstpaths.nstdir, nstpaths.gamename);
	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), moviepath);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), nstpaths.nstdir);
	
	if (gtk_dialog_run(GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		nst_movie_save(filename);
		g_free(filename);
	}
	
	gtk_widget_destroy(dialog);
}

void gtkui_movie_load() {
	// Load a movie from the GUI
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Load movie (.nsv)",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Nestopia movies");
	gtk_file_filter_add_pattern(filter, "*.nsv");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), nstpaths.nstdir);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		nst_movie_load(filename);
		g_free(filename);
	}
	
	gtk_widget_destroy(dialog);
}

void gtkui_movie_stop() {
	nst_movie_stop();
}

void gtkui_cheats_load() {
	// Load cheats from the GUI
	GtkWidget *dialog = gtk_file_chooser_dialog_new("Load cheats (.xml)",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
				NULL);

	GtkFileFilter *filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Nestopia cheats");
	gtk_file_filter_add_pattern(filter, "*.xml");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(dialog), nstpaths.nstdir);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		gtkui_cheats_fill_tree(filename);
		g_free(filename);
	}
	
	gtk_widget_destroy(dialog);
}
