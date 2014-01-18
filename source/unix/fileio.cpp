/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
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

#include <stdlib.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <vector>

//#include <gtk/gtk.h>

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiSound.hpp"
#include "core/api/NstApiInput.hpp"
#include "core/api/NstApiMachine.hpp"
#include "core/api/NstApiUser.hpp"
#include "core/api/NstApiNsf.hpp"
#include "core/api/NstApiMovie.hpp"
#include "core/api/NstApiFds.hpp"
#include "core/api/NstApiCartridge.hpp"
#include "audio.h"
#include "main.h"

#ifndef MINGW
#include <archive.h>
#include <archive_entry.h>
#endif

#define MAX_ITEMS	(512)

extern Nes::Api::Emulator emulator;
//extern GtkWidget *mainwindow;
extern char rootname[512];
extern char msgbuf[512];
extern char nstdir[256];

static std::ifstream *moviePlayFile, *fdsBiosFile, *nstDBFile;
static std::fstream *movieRecFile;

struct archive *a;
struct archive_entry *entry;
int r;

/*static bool run_picker, cancelled;
static GtkTreeStore *treestore;
static GtkTreeIter treeiters[MAX_ITEMS];
static GtkCellRenderer *renderer;
static GtkTreeViewColumn *column;
static GtkTreeSelection *selection;*/

/*static int find_current_selection(void)
{
	int i;

	for (i = 0; i < MAX_ITEMS; i++)
	{
		if (gtk_tree_selection_iter_is_selected(selection, &treeiters[i]))
		{
			return i;
		}
	}

	return -1;
}

void on_archok_clicked(GtkButton *button, gpointer user_data)
{
	run_picker = false;
}

void on_archcancel_clicked(GtkButton *button, gpointer user_data)
{
	run_picker = false;
	cancelled = true;
}

void on_archselect_destroyed(GtkButton *button, gpointer user_data)
{
	run_picker = false;
}

static gint check_list_double(GtkWidget *widget, GdkEventButton *event, gpointer func_data)
{
	if (event->type==GDK_2BUTTON_PRESS)
	{
		run_picker = false;
	}

	return FALSE;
}*/

void fileio_init(void)
{
	moviePlayFile = NULL;
	movieRecFile = NULL;
	fdsBiosFile = NULL;
	nstDBFile = NULL;
}

void fileio_do_state_save(void)
{/*
	Nes::Api::Machine machine( emulator );
	GtkWidget *dialog;
	char defname[512];

	defname[0] = '\0';
	strcpy(defname, rootname);
	strcat(defname, ".nst");

	dialog = gtk_file_chooser_dialog_new ("Save state (.nst)",
					      GTK_WINDOW(mainwindow),
					      GTK_FILE_CHOOSER_ACTION_SAVE,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					      NULL);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), defname);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		std::ofstream stateFile( filename, std::ifstream::out|std::ifstream::binary );

		if (stateFile.is_open())
		{
			machine.SaveState(stateFile);
		}

		g_free (filename);
	}

	gtk_widget_destroy(dialog);*/
}

void fileio_do_state_load(void)
{/*
	Nes::Api::Machine machine( emulator );
	GtkWidget *dialog;
	GtkFileFilter *filter;

	dialog = gtk_file_chooser_dialog_new ("Load state (.nst)",
					      GTK_WINDOW(mainwindow),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Nestopia save states");
	gtk_file_filter_add_pattern(filter, "*.nst");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		std::ifstream stateFile( filename, std::ifstream::in|std::ifstream::binary );

		if (stateFile.is_open())
		{
			machine.LoadState(stateFile);
		}

		g_free (filename);
	}

	gtk_widget_destroy(dialog);*/
}

void fileio_do_movie_save(void)
{/*
	Nes::Api::Machine machine( emulator );
	Nes::Api::Movie movie( emulator );
	GtkWidget *dialog;
	char defname[512];

	defname[0] = '\0';
	strcpy(defname, rootname);
	strcat(defname, ".nsv");

	if (movieRecFile)
	{
		delete movieRecFile;
		movieRecFile = NULL;
	}

	dialog = gtk_file_chooser_dialog_new ("Save a movie (.nsv)",
					      GTK_WINDOW(mainwindow),
					      GTK_FILE_CHOOSER_ACTION_SAVE,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_SAVE, GTK_RESPONSE_ACCEPT,
					      NULL);

	gtk_file_chooser_set_current_name(GTK_FILE_CHOOSER(dialog), defname);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		movieRecFile = new std::fstream(filename, std::ifstream::out|std::ifstream::binary); 

		if (movieRecFile->is_open())
		{
			movie.Record((std::iostream&)*movieRecFile, Nes::Api::Movie::CLEAN);
		}
		else
		{
			delete movieRecFile;
			movieRecFile = NULL;
		}

		g_free (filename);
	}

	gtk_widget_destroy(dialog);*/
}

void fileio_do_movie_load(void)
{/*
	Nes::Api::Machine machine( emulator );
	Nes::Api::Movie movie( emulator );
	GtkWidget *dialog;
	GtkFileFilter *filter;

	if (moviePlayFile)
	{
		delete moviePlayFile;
		moviePlayFile = NULL;
	}

	dialog = gtk_file_chooser_dialog_new ("Load a movie (.nsv)",
					      GTK_WINDOW(mainwindow),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "Nestopia movies");
	gtk_file_filter_add_pattern(filter, "*.nsv");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		moviePlayFile = new std::ifstream( filename, std::ifstream::in|std::ifstream::binary ); 

		if (moviePlayFile->is_open())
		{
			movie.Play(*moviePlayFile);
		}
		else
		{
			delete moviePlayFile;
			moviePlayFile = NULL;
		}

		g_free (filename);
	}

	gtk_widget_destroy(dialog);*/
}

void fileio_do_movie_stop(void)
{/*
	Nes::Api::Movie movie( emulator );

	if (movieRecFile || moviePlayFile)
	{
		movie.Stop();
		movie.Eject();

		if (movieRecFile)
		{
			delete movieRecFile;
			movieRecFile = NULL;
		}

		if (moviePlayFile)
		{
			delete moviePlayFile;
			moviePlayFile = NULL;
		}
	}*/
}

void fileio_set_fds_bios(void) {
	
	Nes::Api::Fds fds(emulator);
	char biospath[512];
	
	if (fdsBiosFile) { return; }

	snprintf(biospath, sizeof(biospath), "%sdisksys.rom", nstdir);

	fdsBiosFile = new std::ifstream(biospath, std::ifstream::in|std::ifstream::binary);

	if (fdsBiosFile->is_open())
	{
		fds.SetBIOS(fdsBiosFile);
	}
	else
	{
		snprintf(msgbuf, sizeof(msgbuf), "%s not found, Disk System games will not work.", biospath);
		print_message(msgbuf);
		delete fdsBiosFile;
		fdsBiosFile = NULL;
	}
}

void fileio_shutdown(void)
{
	if (nstDBFile)
	{
		delete nstDBFile;
		nstDBFile = NULL;
	}

	if (fdsBiosFile)
	{
 		delete fdsBiosFile;
		fdsBiosFile = NULL;
	}
}

static int checkExtension(const char *filename)
{
	int nlen;

	nlen = strlen(filename);

	if ((!strcasecmp(&filename[nlen-4], ".nes")) ||
	    (!strcasecmp(&filename[nlen-4], ".fds")) ||
	    (!strcasecmp(&filename[nlen-4], ".nsf")) ||
	    (!strcasecmp(&filename[nlen-4], ".unf")) ||
	    (!strcasecmp(&filename[nlen-5], ".unif"))||
	    (!strcasecmp(&filename[nlen-4], ".xml")))
	{
		return 1;
	}

	return 0;
}

int fileio_load_archive(const char *filename, unsigned char **dataout, int *datasize, int *dataoffset, const char *filetoload, char *outname)
{
	FILE *f;
	unsigned char idbuf[4];
	int filesFound = 0;
	std::vector<char *> filelist;	// list of files we can load in this archive

	// default case: outname is filename
	if (outname)
	{
		strcpy(outname, filename);
	}

	f = fopen(filename, "rb");

	if (!f)
	{
		return 0;	// no good
	}

	fread(idbuf, 4, 1, f);
	fclose(f);

//	printf("ID bytes %c %c %x %x\n", idbuf[0], idbuf[1], idbuf[2], idbuf[3]);

// Handle all archives with common libarchive code
	if ((idbuf[0] == 'P') && (idbuf[1] == 'K') && (idbuf[2] == 0x03) && (idbuf[3] == 0x04) || // zip
		((idbuf[0] == '7') && (idbuf[1] == 'z') && (idbuf[2] == 0xbc) && (idbuf[3] == 0xaf)) || // 7zip
		((idbuf[0] == 0xfd) && (idbuf[1] == 0x37) && (idbuf[2] == 0x7a) && (idbuf[3] == 0x58)) || // txz
		((idbuf[0] == 0x1f) && (idbuf[1] == 0x8b) && (idbuf[2] == 0x08) && (idbuf[3] == 0x00)) || // tgz
		((idbuf[0] == 0x42) && (idbuf[1] == 0x5a) && (idbuf[2] == 0x68) && (idbuf[3] == 0x39)) // tbz
	) {
	#ifndef MINGW
		a = archive_read_new();
		archive_read_support_filter_all(a);
		archive_read_support_format_all(a);
		r = archive_read_open_filename(a, filename, 10240);
		
		int64_t entry_size;

		if (r != ARCHIVE_OK) {
			print_message("Archive failed to open.");
		}
		
		while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
			const char *currentFile = archive_entry_pathname(entry);
			unsigned char *fileContents;
			entry_size = archive_entry_size(entry);
			fileContents = (unsigned char *)malloc(entry_size);
			
			if (filetoload != NULL) {
				if (!strcmp(currentFile, filetoload)) {
					archive_read_data(a, fileContents, entry_size);
					archive_read_data_skip(a);
					r = archive_read_free(a);
				
					*datasize = entry_size;
					*dataout = fileContents;
					*dataoffset = 0;
					return 1;
				}
			}
			
			else {
				if (checkExtension(currentFile))
				{
					char *tmpstr;

					tmpstr = (char *)malloc(strlen(currentFile)+1);
					strcpy(tmpstr, currentFile);

					// add to the file list
					filelist.push_back(tmpstr);
					filesFound++;
				}
			}
		}
	#endif
	}
	
	else if ((idbuf[0] == 'R') && (idbuf[1] == 'a') && (idbuf[2] == 'r') && (idbuf[3] == '!')) 
	{	// it's rar 
		print_message("Rar files are not supported.");
	}

	// if we found any files and weren't forced to load them, handle accordingly
	if (filesFound)
	{
		// only 1 file found, just run it
		if (filesFound == 1)
		{
			char fname[512];
			
			strcpy(fname, filelist[0]);

			free(filelist[0]);
			filelist.clear();

			strcpy(outname, fname);

			return fileio_load_archive(filename, dataout, datasize, dataoffset, fname, NULL); 
		}
		/*else	// multiple files we can handle found, give the user a choice
		{
			int sel;
			char fname[512];

			GtkWidget *archselect = gtk_window_new(GTK_WINDOW_TOPLEVEL);
			gtk_window_set_title(GTK_WINDOW (archselect), "Pick game in archive");
			gtk_window_set_modal(GTK_WINDOW (archselect), TRUE);
			
			GtkWidget *archbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
			gtk_container_add(GTK_CONTAINER(archselect), archbox);
			gtk_widget_show(archbox);

			GtkWidget *scrolledwindow = gtk_scrolled_window_new(NULL, NULL);
			gtk_box_pack_start(GTK_BOX(archbox), scrolledwindow, TRUE, TRUE, 0);
			gtk_widget_set_size_request(scrolledwindow, 340, 340);
			gtk_widget_show(scrolledwindow);

			GtkWidget *buttonbox = gtk_widget_new(GTK_TYPE_BOX, "halign", GTK_ALIGN_END, NULL);
			gtk_box_pack_start(GTK_BOX(archbox), buttonbox, FALSE, TRUE, 0);
			gtk_widget_show(buttonbox);

			GtkWidget *archtree = gtk_tree_view_new();
			gtk_container_add(GTK_CONTAINER (scrolledwindow), archtree);
			gtk_tree_view_set_fixed_height_mode(GTK_TREE_VIEW (archtree), FALSE);
			g_signal_connect(G_OBJECT(archtree), "button_press_event", G_CALLBACK(check_list_double), NULL);
			gtk_widget_show(archtree);

			// set up our tree store
			treestore = gtk_tree_store_new(1, G_TYPE_STRING);

			// attach the store to the tree	
			gtk_tree_view_set_model(GTK_TREE_VIEW(archtree), GTK_TREE_MODEL(treestore));
			
			for (int fn = 0; fn < filelist.size(); fn++)
			{
				gtk_tree_store_insert(treestore, &treeiters[fn], NULL, 999999);
				gtk_tree_store_set(treestore, &treeiters[fn], 0, filelist[fn], -1);
			}

			// create a cell renderer using the stock text one
			renderer = gtk_cell_renderer_text_new();

			// create a display column using the renderer
			column = gtk_tree_view_column_new_with_attributes ("NES file",
		                                                   renderer,
		                                                   "text", 0,
		                                                   NULL);

			// add the display column and renderer to the tree view
			gtk_tree_view_append_column(GTK_TREE_VIEW (archtree), column);
			
			GtkWidget *archcancel = gtk_widget_new(GTK_TYPE_BUTTON, "label", GTK_STOCK_CANCEL, "halign", GTK_ALIGN_END, "margin-top", 8, "margin-bottom", 8, "margin-right", 8, NULL);
			gtk_button_set_use_stock(GTK_BUTTON(archcancel), TRUE);
			gtk_box_pack_start(GTK_BOX(buttonbox), archcancel, FALSE, FALSE, 0);
			gtk_widget_show(archcancel);

			GtkWidget *archok = gtk_widget_new(GTK_TYPE_BUTTON, "label", GTK_STOCK_OK, "halign", GTK_ALIGN_END, "margin-top", 8, "margin-bottom", 8, "margin-right", 8, NULL);
			gtk_button_set_use_stock(GTK_BUTTON(archok), TRUE);
			gtk_box_pack_start(GTK_BOX(buttonbox), archok, FALSE, FALSE, 0);
			gtk_widget_show(archok);

			// get the selection object too
			selection = gtk_tree_view_get_selection(GTK_TREE_VIEW(archtree));

			g_signal_connect(G_OBJECT(archcancel), "clicked",
				G_CALLBACK(on_archcancel_clicked), NULL);

			g_signal_connect(G_OBJECT(archok), "clicked",
				G_CALLBACK(on_archok_clicked), NULL);
				
			g_signal_connect(G_OBJECT(archselect), "destroy",
				G_CALLBACK(on_archselect_destroyed), NULL);

			gtk_widget_show(archselect);
			
			run_picker = true;
			cancelled = false;

			while (run_picker)
			{
				gtk_main_iteration_do(TRUE);
			}

			sel = find_current_selection();

			gtk_widget_destroy(archselect);

			// was something picked?
			if ((sel != -1) && (!cancelled))
			{
				strcpy(fname, filelist[sel]);
			}

			// free all the temp filenames
			for (int fn = 0; fn < filelist.size(); fn++)
			{
				free(filelist[fn]);
			}

			// and wipe the vector
			filelist.clear();

			if ((sel != -1) && (!cancelled))
			{
				if (outname)
				{
					strcpy(outname, fname);
				}

				return fileio_load_archive(filename, dataout, datasize, dataoffset, fname, NULL); 
			}
		}*/
	}

	return 0;
}

void fileio_load_db(void) {
	
	Nes::Api::Cartridge::Database database(emulator);
	char dbpath[512];

	if (nstDBFile) { return; }

	// Try to open the database file
	snprintf(dbpath, sizeof(dbpath), "%sNstDatabase.xml", nstdir);
	nstDBFile = new std::ifstream(dbpath, std::ifstream::in|std::ifstream::binary);
	
	if (nstDBFile->is_open()) {
		database.Load(*nstDBFile);
		database.Enable(true);
		return;
	}
#ifndef MINGW
	// If it fails, try looking in the data directory
	snprintf(dbpath, sizeof(dbpath), "%s/NstDatabase.xml", DATADIR);
	nstDBFile = new std::ifstream(dbpath, std::ifstream::in|std::ifstream::binary);
	
	if (nstDBFile->is_open()) {
		database.Load(*nstDBFile);
		database.Enable(true);
		return;
	}
	
	// If that fails, try looking in the working directory
	char *pwd = getenv("PWD");
	snprintf(dbpath, sizeof(dbpath), "%s/NstDatabase.xml", pwd);
	nstDBFile = new std::ifstream(dbpath, std::ifstream::in|std::ifstream::binary);
	
	if (nstDBFile->is_open()) {
		database.Load(*nstDBFile);
		database.Enable(true);
		return;
	}
#endif
	else {
		print_message("NstDatabase.xml not found!");
		delete nstDBFile;
		nstDBFile = NULL;
	}
}

