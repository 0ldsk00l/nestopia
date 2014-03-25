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

#include "../main.h"
 
#include "gtkui.h"
 
extern GtkWidget *gtkwindow;

void gtkui_file_open(GtkButton *button, gpointer user_data) {
	// Open a file using a GTK+ dialog
	GtkWidget *dialog;
	GtkFileFilter *filter;

	dialog = gtk_file_chooser_dialog_new(
				"Select a ROM",
				GTK_WINDOW(gtkwindow),
				GTK_FILE_CHOOSER_ACTION_OPEN,
				GTK_STOCK_CANCEL,
				GTK_RESPONSE_CANCEL,
				GTK_STOCK_OPEN,
				GTK_RESPONSE_ACCEPT,
				NULL);
	
	filter = gtk_file_filter_new();
	
	gtk_file_filter_set_name(filter, "NES ROMs");
	gtk_file_filter_add_pattern(filter, "*.nes");
	gtk_file_filter_add_pattern(filter, "*.fds");
	gtk_file_filter_add_pattern(filter, "*.unf");
	gtk_file_filter_add_pattern(filter, "*.unif");
	/*gtk_file_filter_add_pattern(filter, "*.nsf");
	gtk_file_filter_add_pattern(filter, "*.zip");
	gtk_file_filter_add_pattern(filter, "*.7z");
	gtk_file_filter_add_pattern(filter, "*.txz");
	gtk_file_filter_add_pattern(filter, "*.tar.xz");
	gtk_file_filter_add_pattern(filter, "*.tgz");
	gtk_file_filter_add_pattern(filter, "*.tar.gz");
	gtk_file_filter_add_pattern(filter, "*.tbz");
	gtk_file_filter_add_pattern(filter, "*.tar.bz2");*/
	
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);
	
	if (gtk_dialog_run(GTK_DIALOG(dialog)) == GTK_RESPONSE_ACCEPT) {
		char *filename;
		filename = gtk_file_chooser_get_filename(GTK_FILE_CHOOSER(dialog));
		nst_load(filename);
		g_free(filename);
	}
	
	gtk_widget_destroy(dialog);
}
