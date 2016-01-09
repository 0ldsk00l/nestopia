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

#include <iostream>
#include <fstream>

#include <stdlib.h>

#include "../main.h"
#include "../config.h"
#include "../cheats.h"

#include "gtkui.h"
#include "gtkui_callbacks.h"
#include "gtkui_cheats.h"
#include "gtkui_dialogs.h"

extern settings_t conf;
extern nstpaths_t nstpaths;
extern Emulator emulator;

GtkWidget *cheatwindow;
GtkTreeStore *treestore;
GtkWidget *treeview;
GtkWidget *descedit, *ggedit, *paredit;
GtkWidget *infobar, *infolabel;

Xml savexml;
Xml::Node saveroot;

GtkWidget *gtkui_cheats() {
	// Create the Cheats window
	
	if (cheatwindow) { return NULL; }
	
	cheatwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW (cheatwindow), "Cheat Manager");
	
	GtkWidget *cheatbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(cheatwindow), cheatbox);
	
	GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
	gtk_box_pack_start(GTK_BOX(cheatbox), scrolledwindow, TRUE, TRUE, 0);
	gtk_widget_set_size_request(scrolledwindow, 512, 256);
	
	treeview = gtk_tree_view_new();
	gtk_container_add(GTK_CONTAINER (scrolledwindow), treeview);
	
	infobar = gtk_info_bar_new();
	infolabel = gtk_widget_new(GTK_TYPE_LABEL,"label", "", NULL);
	gtk_box_pack_start(GTK_BOX(cheatbox), infobar, TRUE, TRUE, 0);
	
	GtkWidget *content_area = gtk_info_bar_get_content_area(GTK_INFO_BAR(infobar));
	gtk_box_pack_start(GTK_BOX(content_area), infolabel, TRUE, TRUE, 0);
	
	GtkWidget *opensavebox = gtk_widget_new(GTK_TYPE_BOX, "halign", GTK_ALIGN_END, NULL);
	gtk_box_pack_start(GTK_BOX(cheatbox), opensavebox, FALSE, FALSE, 0);
	
	GtkWidget *cheatopen = gtk_widget_new(
				GTK_TYPE_BUTTON,
				"label", GTK_STOCK_OPEN,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_button_set_use_stock(GTK_BUTTON(cheatopen), TRUE);
	gtk_box_pack_start(GTK_BOX(opensavebox), cheatopen, FALSE, FALSE, 0);
	
	GtkWidget *cheatclear = gtk_widget_new(
				GTK_TYPE_BUTTON,
				"label", GTK_STOCK_CLEAR,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_button_set_use_stock(GTK_BUTTON(cheatclear), TRUE);
	gtk_box_pack_start(GTK_BOX(opensavebox), cheatclear, FALSE, FALSE, 0);
	
	GtkWidget *cheatremove = gtk_widget_new(
				GTK_TYPE_BUTTON,
				"label", GTK_STOCK_REMOVE,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_button_set_use_stock(GTK_BUTTON(cheatremove), TRUE);
	gtk_box_pack_start(GTK_BOX(opensavebox), cheatremove, FALSE, FALSE, 0);
	
	GtkWidget *descbox = gtk_widget_new(
				GTK_TYPE_BOX,
				"halign", GTK_ALIGN_END,
				NULL);
	gtk_box_pack_start(GTK_BOX(cheatbox), descbox, FALSE, FALSE, 0);
	
	GtkWidget *desclabel = gtk_widget_new(
				GTK_TYPE_LABEL,
				"label", "Description:",
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-left", 8,
				"margin-right", 8,
				NULL);
	gtk_box_pack_start(GTK_BOX(descbox), desclabel, FALSE, FALSE, 0);
	
	descedit = gtk_widget_new(
				GTK_TYPE_ENTRY,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_box_pack_start(GTK_BOX(descbox), descedit, TRUE, TRUE, 0);
	
	GtkWidget *ggbox = gtk_widget_new(
				GTK_TYPE_BOX,
				"halign", GTK_ALIGN_END,
				NULL);
	gtk_box_pack_start(GTK_BOX(cheatbox), ggbox, FALSE, FALSE, 0);
	
	GtkWidget *gglabel = gtk_widget_new(
				GTK_TYPE_LABEL,
				"label", "Game Genie:",
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-left", 8,
				"margin-right", 8,
				NULL);
	gtk_box_pack_start(GTK_BOX(ggbox), gglabel, FALSE, FALSE, 0);
	
	ggedit = gtk_widget_new(
				GTK_TYPE_ENTRY,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_box_pack_start(GTK_BOX(ggbox), ggedit, TRUE, TRUE, 0);
	
	GtkWidget *genieadd = gtk_widget_new(
				GTK_TYPE_BUTTON,
				"label", GTK_STOCK_ADD,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_button_set_use_stock(GTK_BUTTON(genieadd), TRUE);
	gtk_box_pack_start(GTK_BOX(ggbox), genieadd, FALSE, FALSE, 0);
	
	GtkWidget *parbox = gtk_widget_new(
				GTK_TYPE_BOX,
				"halign", GTK_ALIGN_END,
				NULL);
	gtk_box_pack_start(GTK_BOX(cheatbox), parbox, FALSE, FALSE, 0);
	
	GtkWidget *parlabel = gtk_widget_new(
				GTK_TYPE_LABEL,
				"label", "Pro Action Rocky:",
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-left", 8,
				"margin-right", 8,
				NULL);
	gtk_box_pack_start(GTK_BOX(parbox), parlabel, FALSE, FALSE, 0);
	
	paredit = gtk_widget_new(
				GTK_TYPE_ENTRY,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_box_pack_start(GTK_BOX(parbox), paredit, FALSE, FALSE, 0);
	
	GtkWidget *paradd = gtk_widget_new(
				GTK_TYPE_BUTTON,
				"label", GTK_STOCK_ADD,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_button_set_use_stock(GTK_BUTTON(paradd), TRUE);
	gtk_box_pack_start(GTK_BOX(parbox), paradd, FALSE, FALSE, 0);
	
	GtkWidget *cheatok = gtk_widget_new(
				GTK_TYPE_BUTTON,
				"label", GTK_STOCK_OK,
				"halign", GTK_ALIGN_END,
				"margin-top", 8,
				"margin-bottom", 8,
				"margin-right", 8,
				NULL);
	gtk_button_set_use_stock(GTK_BUTTON(cheatok), TRUE);
	gtk_box_pack_start(GTK_BOX(cheatbox), cheatok, FALSE, FALSE, 0);
	
	gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW(treeview), FALSE);
	
	treestore = gtk_tree_store_new(5, G_TYPE_BOOLEAN, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING, G_TYPE_STRING);
	gtk_tree_view_set_model(GTK_TREE_VIEW(treeview), GTK_TREE_MODEL(treestore));
	
	GtkCellRenderer *renderer = gtk_cell_renderer_text_new();
	GtkCellRenderer *checkbox = gtk_cell_renderer_toggle_new();
	
	GtkTreeViewColumn *column[5];
	// create the display columns
	column[0] = gtk_tree_view_column_new_with_attributes("Enable", checkbox, "active", 0, NULL);
	column[1] = gtk_tree_view_column_new_with_attributes("Game Genie", renderer, "text",  1, NULL);
	column[2] = gtk_tree_view_column_new_with_attributes("PAR", renderer, "text",  2, NULL);
	column[3] = gtk_tree_view_column_new_with_attributes("Raw", renderer, "text",  3, NULL);
	column[4] = gtk_tree_view_column_new_with_attributes("Description", renderer, "text",  4, NULL);

	// add the display column and renderer to the tree view
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column[0]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column[1]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column[2]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column[3]);
	gtk_tree_view_append_column(GTK_TREE_VIEW(treeview), column[4]);

	gtkui_cheats_fill_tree(nstpaths.cheatpath);
	
	/*g_signal_connect(G_OBJECT(checkbox), "toggled",
		G_CALLBACK(gtkui_cheats_check), NULL);*/
	
	g_signal_connect(G_OBJECT(treeview), "row-activated",
		G_CALLBACK(gtkui_cheats_toggle), NULL);
	
	g_signal_connect(G_OBJECT(cheatopen), "clicked",
		G_CALLBACK(gtkui_cheats_load), NULL);
	
	g_signal_connect(G_OBJECT(cheatclear), "clicked",
		G_CALLBACK(gtkui_cheats_clear), NULL);
	
	g_signal_connect(G_OBJECT(cheatremove), "clicked",
		G_CALLBACK(gtkui_cheats_remove), NULL);
	
	g_signal_connect(G_OBJECT(genieadd), "clicked",
		G_CALLBACK(gtkui_cheats_gg_add), NULL);
	
	g_signal_connect(G_OBJECT(paradd), "clicked",
		G_CALLBACK(gtkui_cheats_par_add), NULL);
	
	g_signal_connect(G_OBJECT(cheatok), "clicked",
		G_CALLBACK(gtkui_cheats_ok), NULL);	
	
	g_signal_connect(G_OBJECT(cheatwindow), "destroy",
		G_CALLBACK(gtkui_cheats_ok), NULL);
	
	gtk_widget_show_all(cheatwindow);
	gtk_widget_hide(infobar);

	return cheatwindow;
}

void gtkui_cheats_check(GtkWidget *widget, gchar *element, gpointer userdata) {
	// This function doesn't work. Fix later.
	GtkTreeIter iter;
	
	bool value;
		
	// Read the value of the checkbox
	value = gtk_cell_renderer_toggle_get_active((GtkCellRendererToggle*)widget);
	
	// Flip the value and set it
	value ^= 1;
	gtk_cell_renderer_toggle_set_active((GtkCellRendererToggle*)widget, value);
}

void gtkui_cheats_toggle(GtkWidget *widget, gpointer userdata) {
	// Toggle a cheat on or off
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	
	bool value;
	
	// Get the selected item
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(widget));
	gtk_tree_selection_get_selected(selection, &model, &iter);
	
	// Read the value of the checkbox
	gtk_tree_model_get(model, &iter, 0, &value, -1);
	
	// Flip the value and set it
	value ^= 1;
	gtk_tree_store_set(treestore, &iter, 0, value, -1);
	
	//Re-initialize the cheats
	Cheats cheats(emulator);
	cheats.ClearCodes();
	
	gtk_tree_model_foreach(GTK_TREE_MODEL(model), gtkui_cheats_scan_list, NULL);
}

void gtkui_cheats_fill_tree(char *filename) {
	// Fill the cheat list
	Xml xml;
	
	GtkTreeIter iter;
	
	bool enabled = false;
	
	char codebuf[9];
	char descbuf[512];
	
	gtkui_cheats_clear();
	
	std::ifstream cheatfile(filename, std::ifstream::in|std::ifstream::binary);
	
	if (cheatfile.is_open()) {
		xml.Read(cheatfile);
		
		if (xml.GetRoot().IsType(L"cheats")) {
			
			Xml::Node root(xml.GetRoot());
			Xml::Node node(root.GetFirstChild());
			
			for (int i = 0; i < root.NumChildren(L"cheat"); i++) {
				
				wcstombs(descbuf, node.GetChild(L"description").GetValue(), sizeof(descbuf));
				
				// Check if the cheat is enabled
				node.GetAttribute(L"enabled").IsValue(L"1") ? enabled = true : enabled = false;
				
				// Add the cheats to the list
				if (node.GetChild(L"genie")) { // Game Genie
					wcstombs(codebuf, node.GetChild(L"genie").GetValue(), sizeof(codebuf));
					gtk_tree_store_append(treestore, &iter, NULL);
					gtk_tree_store_set(treestore, &iter,
								0, enabled,
								1, codebuf,
								4, descbuf,
								-1);
					if (enabled) { cheats_code_gg_add(node.GetChild(L"genie").GetValue()); }
				}
				
				else if (node.GetChild(L"rocky")) { // Pro Action Rocky
					wcstombs(codebuf, node.GetChild(L"rocky").GetValue(), sizeof(codebuf));
					gtk_tree_store_append(treestore, &iter, NULL);
					gtk_tree_store_set(treestore, &iter,
								0, enabled,
								2, codebuf,
								4, descbuf,
								-1);
					if (enabled) { cheats_code_par_add(node.GetChild(L"rocky").GetValue()); }
				}
				
				else if (node.GetChild(L"address")) { // Raw
					char rawbuf[11];
					snprintf(rawbuf, sizeof(rawbuf),
								"%04x %02x %02x",
								node.GetChild(L"address").GetUnsignedValue(),
								node.GetChild(L"value").GetUnsignedValue(),
								node.GetChild(L"compare").GetUnsignedValue());
					
					gtk_tree_store_append(treestore, &iter, NULL);
					gtk_tree_store_set(treestore, &iter,
								0, enabled,
								3, rawbuf,
								4, descbuf,
								-1);
					if (enabled) { cheats_code_raw_add(node); }
				}
				
				node = node.GetNextSibling();
			}
		}
		cheatfile.close();
	}	
}

void gtkui_cheats_save() {
	// Save the cheat list
	std::ofstream cheatfile(nstpaths.cheatpath, std::ifstream::out|std::ifstream::binary);
	
	if (cheatfile.is_open()) {
		
		GtkTreeModel *model;
		model = gtk_tree_view_get_model(GTK_TREE_VIEW(treeview));
		
		saveroot = (savexml.GetRoot());
		
		saveroot = savexml.Create( L"cheats" );
		saveroot.AddAttribute( L"version", L"1.0" );
		
		gtk_tree_model_foreach(GTK_TREE_MODEL(model), gtkui_cheats_write_list, NULL);
		
		savexml.Write(saveroot, cheatfile);
		cheatfile.close();
	}
	else { return; }
}

void gtkui_cheats_gg_add(GtkWidget *widget, gpointer userdata) {
	// Add a Game Genie code to the list
	GtkTreeIter iter;
	
	Cheats cheats(emulator);
	Cheats::Code code;
	
	char codebuf[9];
	char descbuf[512];
	
	snprintf(codebuf, sizeof(codebuf), "%.8s", gtk_entry_get_text(GTK_ENTRY(ggedit)));
	snprintf(descbuf, sizeof(descbuf), "%s", gtk_entry_get_text(GTK_ENTRY(descedit)));
	
	if (cheats.GameGenieDecode(codebuf, code) == Nes::RESULT_OK) {
		gtk_tree_store_append(treestore, &iter, NULL);
		gtk_tree_store_set(treestore, &iter,
					0, true,
					1, codebuf,
					4, descbuf,
					-1);
		gtk_entry_set_text(GTK_ENTRY(descedit), "");
		gtk_entry_set_text(GTK_ENTRY(ggedit), "");
		gtk_entry_set_text(GTK_ENTRY(paredit), "");
		gtk_widget_hide(infobar);
		gtk_label_set_text(GTK_LABEL(infolabel), "");
		cheats.SetCode(code);
	}
	else {
		gtk_info_bar_set_message_type(GTK_INFO_BAR(infobar), GTK_MESSAGE_ERROR);
		gtk_label_set_text(GTK_LABEL(infolabel), "Error: Invalid Game Genie code");
		gtk_widget_show(infobar);
	}
}

void gtkui_cheats_par_add(GtkWidget *widget, gpointer userdata) {
	// Add a Pro Action Rocky code to the list
	GtkTreeIter iter;
	
	Cheats cheats(emulator);
	Cheats::Code code;
	
	char codebuf[9];
	char descbuf[512];
	
	snprintf(codebuf, sizeof(codebuf), "%.8s", gtk_entry_get_text(GTK_ENTRY(paredit)));
	snprintf(descbuf, sizeof(descbuf), "%s", gtk_entry_get_text(GTK_ENTRY(descedit)));
	
	if (cheats.ProActionRockyDecode(codebuf, code) == Nes::RESULT_OK) {
		gtk_tree_store_append(treestore, &iter, NULL);
		gtk_tree_store_set(treestore, &iter,
					0, true,
					1, codebuf,
					4, descbuf,
					-1);
		gtk_entry_set_text(GTK_ENTRY(descedit), "");
		gtk_entry_set_text(GTK_ENTRY(ggedit), "");
		gtk_entry_set_text(GTK_ENTRY(paredit), "");
		gtk_widget_hide(infobar);
		gtk_label_set_text(GTK_LABEL(infolabel), "");
		cheats.SetCode(code);
	}
	else {
		gtk_info_bar_set_message_type(GTK_INFO_BAR(infobar), GTK_MESSAGE_ERROR);
		gtk_label_set_text(GTK_LABEL(infolabel), "Error: Invalid PAR code");
		gtk_widget_show(infobar);
	}
}

void gtkui_cheats_remove(GtkWidget *widget, gpointer userdata) {
	// Remove a cheat from the list
	GtkTreeIter iter;
	GtkTreeModel *model;
	GtkTreeSelection *selection;
	
	// Get the selected item
	selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(treeview));
	gtk_tree_selection_get_selected(selection, &model, &iter);

	// Remove the cheat
	if (gtk_tree_store_iter_is_valid(treestore, &iter)) {
		gtk_tree_store_remove(treestore, &iter);
	}
	
	//Re-initialize the cheats
	Cheats cheats(emulator);
	cheats.ClearCodes();
	
	gtk_tree_model_foreach(GTK_TREE_MODEL(model), gtkui_cheats_scan_list, NULL);
}

void gtkui_cheats_ok() {
	// Save the cheats and close the window
	gtkui_cheats_save();
	gtk_widget_destroy(cheatwindow);
	cheatwindow = NULL;
}

void gtkui_cheats_clear() {
	// Clear the list
	gtk_tree_store_clear(treestore);
	Cheats cheats(emulator);
	cheats.ClearCodes();
}

gboolean gtkui_cheats_scan_list(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata) {
	// Scan through the list of cheats
	Cheats cheats(emulator);
	Cheats::Code code;
	
	bool enabled;
	gchar *ggcode, *parcode, *rawcode, *description;
	
	gtk_tree_model_get(model, iter, 0, &enabled, 1, &ggcode, 2, &parcode, 3, &rawcode, 4, &description, -1);
	
	if (enabled) {
		if (ggcode) {
			cheats.GameGenieDecode(ggcode, code);
			cheats.SetCode(code);
		}
		else if (parcode) {
			cheats.ProActionRockyDecode(parcode, code);
			cheats.SetCode(code);
		}
		else if (rawcode) {
			code.useCompare = false;
			
			int addr, value, compare;
			char buf[5];
			
			snprintf(buf, sizeof(buf), "%c%c%c%c\0", rawcode[0], rawcode[1], rawcode[2], rawcode[3]);
			sscanf(buf, "%x", &addr);
			
			snprintf(buf, sizeof(buf), "%c%c\0", rawcode[5], rawcode[6]);
			sscanf(buf, "%x", &value);
			
			snprintf(buf, sizeof(buf), "%c%c\0", rawcode[8], rawcode[9]);
			sscanf(buf, "%x", &compare);
			
			code.address = addr;
			code.value = value;
			code.compare = compare;
			
			if (compare) { code.useCompare = true; }
			
			cheats.SetCode(code);
		}
	}
	
	g_free(ggcode);
	g_free(parcode);
	g_free(rawcode);
	g_free(description);
	
	return false;
}

gboolean gtkui_cheats_write_list(GtkTreeModel *model, GtkTreePath *path, GtkTreeIter *iter, gpointer userdata) {
	// Write entries to the cheat file
	bool enabled;
	gchar *ggcode, *parcode, *rawcode, *description;
	
	char buf[9];
	wchar_t wbuf[9];
	
	gtk_tree_model_get(model, iter, 0, &enabled, 1, &ggcode, 2, &parcode, 3, &rawcode, 4, &description, -1);
	
	Xml::Node node(saveroot.AddChild(L"cheat"));
	node.AddAttribute(L"enabled", enabled ? L"1" : L"0");
	
	if (ggcode) {
		snprintf(buf, sizeof(buf), "%s", ggcode);
		mbstowcs(wbuf, buf, 9);
		node.AddChild(L"genie", wbuf);
	}
	if (parcode) {
		snprintf(buf, sizeof(buf), "%s", parcode);
		mbstowcs(wbuf, buf, 9);
		node.AddChild(L"rocky", wbuf);
	}
	if (rawcode) {
		snprintf(buf, sizeof(buf), "0x%c%c%c%c", rawcode[0], rawcode[1], rawcode[2], rawcode[3]);
		mbstowcs(wbuf, buf, 9);
		node.AddChild(L"address", wbuf);
		
		snprintf(buf, sizeof(buf), "0x%c%c", rawcode[5], rawcode[6]);
		mbstowcs(wbuf, buf, 9);
		node.AddChild(L"value", wbuf);
		
		snprintf(buf, sizeof(buf), "0x%c%c", rawcode[8], rawcode[9]);
		mbstowcs(wbuf, buf, 9);
		node.AddChild(L"compare", wbuf);
	}
	if (description) {
		char descbuf[512];
		wchar_t wdescbuf[512];
		
		snprintf(descbuf, sizeof(descbuf), "%s", description);
		mbstowcs(wdescbuf, descbuf, 512);
		node.AddChild(L"description", wdescbuf);
	}
		
	g_free(ggcode);
	g_free(parcode);
	g_free(rawcode);
	g_free(description);
	
	return false;
}
