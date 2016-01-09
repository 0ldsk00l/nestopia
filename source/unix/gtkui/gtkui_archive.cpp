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

#include <string.h>
#include <archive.h>
#include <archive_entry.h>

#include "../main.h"
#include "../config.h"

#include "gtkui.h"
#include "gtkui_archive.h"

bool windowopen, cancelled;

GtkWidget *archivewindow;

extern settings_t conf;

bool gtkui_archive_handle(const char *filename, char *reqfile, size_t reqsize) {
	// Select a filename to pull out of the archive
	struct archive *a;
	struct archive_entry *entry;
	int r, numarchives = 0;
	
	cancelled = false;
	
	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, filename, 10240);
	
	// Test if it's actually an archive
	if (r != ARCHIVE_OK) {
		r = archive_read_free(a);
		return false;
	}
	// If it is an archive, handle it
	else {
		// Don't try to bring up a GUI selector if the GUI is disabled
		if (conf.misc_disable_gui) {
			// Fill the treestore with the filenames
			while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
				const char *currentfile = archive_entry_pathname(entry);
				if (nst_archive_checkext(currentfile)) {
					snprintf(reqfile, reqsize, "%s", currentfile);
					break;
				}
			}
			archive_read_data_skip(a);
			// Free the archive
			r = archive_read_free(a);
			return true;
		}
		
		// Set up the archive window
		GtkTreeIter iter;
		GtkTreeModel *model;
		GtkTreeSelection *selection;
		
		archivewindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
		gtk_window_set_title(GTK_WINDOW(archivewindow), "Choose File from Archive");
		gtk_window_set_modal(GTK_WINDOW(archivewindow), TRUE);
		
		GtkWidget *archivebox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
		gtk_container_add(GTK_CONTAINER(archivewindow), archivebox);
		gtk_widget_show(archivebox);
		
		GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
		gtk_box_pack_start(GTK_BOX(archivebox), scrolledwindow, TRUE, TRUE, 0);
		gtk_widget_set_size_request(scrolledwindow, 340, 340);
		gtk_widget_show(scrolledwindow);
		
		GtkWidget *buttonbox = gtk_widget_new(GTK_TYPE_BOX, "halign", GTK_ALIGN_END, NULL);
		gtk_box_pack_start(GTK_BOX(archivebox), buttonbox, FALSE, TRUE, 0);
		gtk_widget_show(buttonbox);
		
		GtkWidget *treeview = gtk_tree_view_new();
		gtk_container_add(GTK_CONTAINER(scrolledwindow), treeview);
		gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW (treeview), FALSE);
		gtk_widget_show(treeview);
		
		GtkTreeStore *treestore = gtk_tree_store_new(1, G_TYPE_STRING);
		
		gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(treestore));
		
		// Fill the treestore with the filenames
		while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
			const char *currentfile = archive_entry_pathname(entry);
			if (nst_archive_checkext(currentfile)) {
				gtk_tree_store_append(treestore, &iter, NULL);
				gtk_tree_store_set(treestore, &iter, 0, currentfile, -1);
				numarchives++;
				snprintf(reqfile, reqsize, "%s", currentfile);
			}
			archive_read_data_skip(a);
		}
		// Free the archive
		r = archive_read_free(a);
		
		// If there are no valid files in the archive, return
		if (numarchives == 0) {	return false; }
		// If there's only one file, don't bring up the selector
		else if (numarchives == 1) { return true; }
		
		GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
		
		GtkTreeViewColumn *column = gtk_tree_view_column_new_with_attributes(
				"NES file",
				renderer,
				"text", 0,
				NULL);
		
		gtk_tree_view_append_column(GTK_TREE_VIEW (treeview), column);
		
		GtkWidget *cancelbutton = gtk_widget_new(
					GTK_TYPE_BUTTON,
					"label", GTK_STOCK_CANCEL,
					"halign", GTK_ALIGN_END,
					"margin-top", 8,
					"margin-bottom", 8,
					"margin-right", 8,
					NULL);
		gtk_button_set_use_stock(GTK_BUTTON(cancelbutton), TRUE);
		gtk_box_pack_start(GTK_BOX(buttonbox), cancelbutton, FALSE, FALSE, 0);
		gtk_widget_show(cancelbutton);
		
		GtkWidget *okbutton = gtk_widget_new(
					GTK_TYPE_BUTTON,
					"label", GTK_STOCK_OK,
					"halign", GTK_ALIGN_END,
					"margin-top", 8,
					"margin-bottom", 8,
					"margin-right", 8,
					NULL);
		gtk_button_set_use_stock(GTK_BUTTON(okbutton), TRUE);
		gtk_box_pack_start(GTK_BOX(buttonbox), okbutton, FALSE, FALSE, 0);
		gtk_widget_show(okbutton);
		
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		
		g_signal_connect(G_OBJECT(okbutton), "clicked",
			G_CALLBACK(gtkui_archive_ok), NULL);
			
		g_signal_connect(G_OBJECT(cancelbutton), "clicked",
			G_CALLBACK(gtkui_archive_cancel), NULL);
		
		g_signal_connect(G_OBJECT(treeview), "row-activated",
			G_CALLBACK(gtkui_archive_ok), NULL);
			
		g_signal_connect(G_OBJECT(archivewindow), "destroy",
			G_CALLBACK(gtkui_archive_cancel), NULL);
		
		gtk_widget_show(archivewindow);
		
		// Freeze the rest of the program until a selection is made
		windowopen = true;
		while (windowopen) {
			gtk_main_iteration_do(TRUE);
			if (cancelled) { return false; }
		}
		
		gchar *reqbuf;
		selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
		gtk_tree_selection_get_selected(selection, &model, &iter);
		gtk_tree_model_get(model, &iter, 0, &reqbuf, -1);
		
		gtk_widget_destroy(archivewindow);
		
		snprintf(reqfile, reqsize, "%s", reqbuf);
		return true;
	}
	return false;
}

void gtkui_archive_ok() {
	windowopen = false;
}

void gtkui_archive_cancel() {
	cancelled = true;
	gtk_widget_destroy(archivewindow);
}
