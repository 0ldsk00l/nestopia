/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
 * Copyright (C) 2012-2013 R. Danbrook
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
#include <unistd.h>
#include <SDL.h>
#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <iomanip>
#include <string.h>
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>

#include <gtk/gtk.h>
#include <gdk/gdkkeysyms.h>
//#include <gdk/gdkx.h>

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiSound.hpp"
#include "core/api/NstApiInput.hpp"
#include "core/api/NstApiMachine.hpp"
#include "core/api/NstApiUser.hpp"
#include "core/api/NstApiNsf.hpp"
#include "core/api/NstApiMovie.hpp"
#include "core/api/NstApiFds.hpp"
#include "core/api/NstApiRewinder.hpp"
#include "core/api/NstApiCartridge.hpp"
#include "core/api/NstApiCheats.hpp"
#include "core/NstCrc32.hpp"
#include "core/NstChecksum.hpp"
#include "core/NstXml.hpp"

#include "main.h"
#include "gtkui.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "fileio.h"
#include "cheats.h"
#include "config.h"
#include "seffect.h"

using namespace Nes::Api;
using namespace LinuxNst;

extern Emulator emulator;
extern settings *conf;
extern SDL_Window *sdlwindow;

static CheatMgr *sCheatMgr;

static GdkPixbuf *app_icon;

GtkWidget *mainwindow;
GtkWidget *configwindow;
GtkWidget *messagewindow;
GtkWidget *drawingarea;
GtkWidget *statusbar;

GdkColor bg = {0, 0, 0, 0};

static GtkWidget *nsfplayer, *nsftitle, *nsfauthor, *nsfmaker, *text_volume, *scroll_volume;

static char volumestr[5], surrmulstr[5];

char windowid[24];

int playernumber = 0;

extern int schedule_stop;
extern dimensions rendersize;

bool wasplaying = 0;

void on_videocombo_changed(GtkComboBox *combobox, gpointer user_data) {
	conf->misc_video_region = gtk_combo_box_get_active(combobox);
}

void on_mainwindow_destroy(GObject *object, gpointer user_data)
{
	gtk_widget_destroy(mainwindow);
	NstScheduleQuit();
}

static void set_cur_path(char *inpath)
{
	char cwd[1024];
	int i;

	strcpy(cwd, inpath);

	i = strlen(inpath)-1;

	while ((cwd[i] != '/') && (i > 0))
	{
		i--;
	}

	cwd[i+1] = '\0';

	chdir(cwd);
}

void on_open_clicked(GtkButton *button, gpointer user_data)
{
	GtkWidget *dialog;
	GtkFileFilter *filter;

	if (NstIsPlaying())
	{
		schedule_stop = 1;
	}
	
	dialog = gtk_file_chooser_dialog_new ("Select a ROM",
						GTK_WINDOW(mainwindow),
							GTK_FILE_CHOOSER_ACTION_OPEN,
							GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
							GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
							NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "NES ROMs and archives");
	gtk_file_filter_add_pattern(filter, "*.nes");
	gtk_file_filter_add_pattern(filter, "*.fds");
	gtk_file_filter_add_pattern(filter, "*.unf");
	gtk_file_filter_add_pattern(filter, "*.unif");
	gtk_file_filter_add_pattern(filter, "*.nsf");
	gtk_file_filter_add_pattern(filter, "*.zip");
	gtk_file_filter_add_pattern(filter, "*.7z");
	gtk_file_filter_add_pattern(filter, "*.txz");
	gtk_file_filter_add_pattern(filter, "*.tar.xz");
	gtk_file_filter_add_pattern(filter, "*.tgz");
	gtk_file_filter_add_pattern(filter, "*.tar.gz");
	gtk_file_filter_add_pattern(filter, "*.tbz");
	gtk_file_filter_add_pattern(filter, "*.tar.bz2");

	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		//int i;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// make sure open starts in this location next time
		set_cur_path(filename);

		wasplaying = 1;
		
		// load the cartridge
		NstLoadGame(filename);
		g_free (filename);
	}

	gtk_widget_destroy(dialog);

	if (wasplaying) {
		NstPlayGame();
	}
}

void on_playbutton_clicked(GtkButton *button,  gpointer user_data)
{
	NstPlayGame();
}

void okbutton_clicked() {
	gtk_widget_destroy(configwindow);
}

void configwindow_destroyed() {
	if (wasplaying) {
		NstPlayGame();
	}
	else {
		//get_screen_res();
	}

	redraw_drawingarea(rendersize.w, rendersize.h);
	SDL_SetWindowSize(sdlwindow, rendersize.w, rendersize.h);
}

void on_check_blendpix_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	conf->video_xbr_pixel_blending = gtk_toggle_button_get_active(togglebutton);
}

void on_check_fullscreen_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	conf->video_fullscreen = gtk_toggle_button_get_active(togglebutton);
}

void on_check_fsnativeres_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	conf->video_preserve_aspect = gtk_toggle_button_get_active(togglebutton);
}

void on_check_tvaspect_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	conf->video_tv_aspect = gtk_toggle_button_get_active(togglebutton);
}

void on_check_oscanmask_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	conf->video_mask_overscan = gtk_toggle_button_get_active(togglebutton);
}

void on_unlimitsprcheck_toggled (GtkToggleButton *togglebutton, gpointer user_data) {
	conf->video_unlimited_sprites = gtk_toggle_button_get_active(togglebutton);
}

void on_scalecombo_changed(GtkComboBox *combobox, gpointer user_data) {
	conf->video_filter = gtk_combo_box_get_active(combobox);
}

void on_stereocheck_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	
	Sound sound( emulator );

	conf->audio_stereo = gtk_toggle_button_get_active(togglebutton);

	if (NstIsPlaying())
	{
		if (conf->audio_stereo)
		{
			sound.SetSpeaker( Sound::SPEAKER_STEREO );
		}
		else
		{
			sound.SetSpeaker( Sound::SPEAKER_MONO );
		}
	}
}

void on_surrcheck_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	conf->audio_surround = gtk_toggle_button_get_active(togglebutton); 
}

void on_excitecheck_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	conf->audio_stereo_exciter = gtk_toggle_button_get_active(togglebutton); 
}

void on_ratecombo_changed(GtkComboBox *combobox, gpointer user_data) {
	
	switch(gtk_combo_box_get_active(combobox)) {
		case 0:
			conf->audio_sample_rate = 11025;
			break;
		case 1:
			conf->audio_sample_rate = 22050;
			break;
		case 2:
			conf->audio_sample_rate = 44100;
			break;
		case 3:
			conf->audio_sample_rate = 48000;
			break;
		default: break;
	}
}

void on_ratecombo_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data)
{
}

void on_rendercombo_changed (GtkComboBox *combobox, gpointer user_data) {
	conf->video_renderer = gtk_combo_box_get_active(combobox);
}

void playercombo_changed(GtkComboBox *combobox, gpointer user_data) {
	playernumber = gtk_combo_box_get_active(combobox);
}

void on_systemcombo_changed(GtkComboBox *combobox, gpointer user_data) {
	conf->misc_default_system = gtk_combo_box_get_active(combobox);
}

void on_scaleamtcombo_changed(GtkComboBox *combobox, gpointer user_data) {
	conf->video_scale_factor = gtk_combo_box_get_active(combobox) + 1;
}

// FIX
void on_configcombo_changed(GtkComboBox *combobox, gpointer user_data) {
	/*sSettings->SetConfigItem(gtk_combo_box_get_active(combobox));
	int curItem = sSettings->GetConfigItem();*/
}

void on_spatchcombo_changed(GtkComboBox *combobox, gpointer user_data) {
	conf->misc_soft_patching = gtk_combo_box_get_active(combobox);
}

void on_sndapicombo_changed(GtkComboBox *combobox, gpointer user_data) {
	conf->audio_api = gtk_combo_box_get_active(combobox);
}

void on_volumescroll_value_changed(GtkRange *range, gpointer user_data) {
	Sound sound( emulator );

	conf->audio_volume = (int)gtk_range_get_value(range);
	//sprintf(volumestr, "%d", sSettings->GetVolume());
	//gtk_label_set_text(GTK_LABEL(text_volume), volumestr);

	if (NstIsPlaying())
	{
		sound.SetVolume(Sound::ALL_CHANNELS, conf->audio_volume);
	}
}

void on_surrscroll_value_changed(GtkRange *range, gpointer user_data) {
	conf->audio_surround_multiplier = (int)gtk_range_get_value(range);
	//sprintf(surrmulstr, "%d", sSettings->GetSurrMult());
	//gtk_label_set_text(GTK_LABEL(text_surround), surrmulstr);
}

// FIX
void on_cheatbutton_pressed(GtkButton *button, gpointer user_data)
{
	sCheatMgr->ShowManager();
}

void on_aboutbutton_clicked(GtkButton *button,  gpointer user_data)
{
	create_about();
}

void on_volumescroll_configure_event(GtkWidget *widget, GdkEventConfigure *event, gpointer user_data) {

}

void on_ntsccombo_changed (GtkComboBox *combobox, gpointer user_data) {
	conf->video_ntsc_mode = gtk_combo_box_get_active(combobox);
}

void on_xbrcombo_changed (GtkComboBox *combobox, gpointer user_data) {
	conf->video_xbr_corner_rounding = gtk_combo_box_get_active(combobox);
}

void on_configbutton_clicked(GtkButton *button, gpointer user_data)
{
	//NstLaunchConfig();
}

void inputcfg_clicked(GtkButton *button, int data) {

	/*if (playernumber == 0 && data < 16) {	// Player 1
		switch(data) {
			case 0:
				sSettings->SetConfigItem(0); //P1UP
				break;
			case 1:
				sSettings->SetConfigItem(1); //P1DN
				break;
			case 2:
				sSettings->SetConfigItem(2); //P1LT
				break;
			case 3:
				sSettings->SetConfigItem(3); //P1RT
				break;
			case 4:
				sSettings->SetConfigItem(4); //P1A
				break;
			case 5:
				sSettings->SetConfigItem(5); //P1B
				break;
			case 6:
				sSettings->SetConfigItem(6); //P1START
				break;
			case 7:
				sSettings->SetConfigItem(7); //P1SELECT
				break;
		}
	}
	else if (playernumber == 1 && data < 16) {	// Player 2
		switch(data) {
			case 0:
				sSettings->SetConfigItem(8); //P2UP
				break;
			case 1:
				sSettings->SetConfigItem(9); //P2DN
				break;
			case 2:
				sSettings->SetConfigItem(10); //P2LT
				break;
			case 3:
				sSettings->SetConfigItem(11); //P2RT
				break;
			case 4:
				sSettings->SetConfigItem(12); //P2A
				break;
			case 5:
				sSettings->SetConfigItem(13); //P2B
				break;
			case 6:
				sSettings->SetConfigItem(14); //P2START
				break;
			case 7:
				sSettings->SetConfigItem(15); //P2SELECT
				break;
		}
	}
	else {
		switch(data) {
			case 26:
				sSettings->SetConfigItem(26); //Rewind Start
				break;
			case 27:
			sSettings->SetConfigItem(27); //Rewind Start
				break;
		}
	}
	NstLaunchConfig();*/
}

/*void on_nsfplayer_destroy(GObject *object, gpointer user_data)
{
	NstStopNsf();
	//gtk_widget_destroy(nsfplayer);
}*/

void load_file_by_uri(char *filename) {
	char *fsub;
	char transname[512], convstr[3];
	int i, j;

	// make sure the URI header is there
	if (strncmp(filename, "file://", 7))
	{
		return;
	}

	// skip the URI header
	filename += 7;

	// remove the path component
	fsub = filename + strlen(filename)-1;
	while (*fsub != '/')
	{
		fsub--;
	}
	fsub++;	// skip the slash

	// un-escape the URI
	i = j = 0;
	while (filename[i] != '\0')
	{
		if (filename[i] == '%')
		{
			char ins;
			char *eptr;

			i++;

			convstr[0] = filename[i++];
			convstr[1] = filename[i++];
			convstr[2] = '\0';

			ins = strtol(convstr, &eptr, 16);

			transname[j++] = ins;
		}
		else
		{
			transname[j++] = filename[i++];
		}
	}
	
	transname[j] = '\0';
	
	// now load it	
	NstLoadGame(transname);
	NstPlayGame();
}

void drag_data_received(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data) {

	if ((widget == NULL) || (dc == NULL)) {
		return;
	}

	if (selection_data == NULL) {
		return;
	}
	
	if (info == 0) {
		gchar *filename = (gchar*)gtk_selection_data_get_data(selection_data);
		int i, datalen;
		char *root;

		// for multiple files in a drag, we get a list of file:// URIs separated by LF/CRs
		// we only accept one in Nestopia
		datalen = strlen(filename);
		root = filename;

		for (i = 0; i < datalen; i++) {
			if ((filename[i] == 0x0d) || (filename[i] == 0x0a)) {
				filename[i] = '\0';

				load_file_by_uri(root);
				return;
			}
		}
	}
}

void gtkui_init(int argc, char *argv[], int xres, int yres)
{
	// crank up our GUI
	mainwindow = create_mainwindow(xres, yres);

	// set up the icon
	char iconpath[1024];
	snprintf(iconpath, sizeof(iconpath), "%s/icons/nestopia.svg", DATADIR);
	
	// Load the icon from local source dir if make install hasn't been done
	struct stat iconstat;
	if (stat(iconpath, &iconstat) == -1) {
		snprintf(iconpath, sizeof(iconpath), "source/unix/icons/nestopia.svg");
	}

	app_icon = gdk_pixbuf_new_from_file(iconpath, NULL);
	gtk_window_set_icon(GTK_WINDOW(mainwindow), app_icon);

	// show the window
	gtk_widget_show(mainwindow);
	
}

/*void UIHelp_NSFLoaded(void)
{
	Nsf nsf( emulator );
	
	create_nsfplayer();

	// show the NSF info
	gtk_label_set_text(GTK_LABEL(nsftitle), nsf.GetName());
	gtk_label_set_text(GTK_LABEL(nsfauthor), nsf.GetArtist());
	gtk_label_set_text(GTK_LABEL(nsfmaker), nsf.GetCopyright());
}*/

// return the icon for alternate windows to use
GdkPixbuf *get_icon() {
	return app_icon;
}

void pause_clicked() {
	bool playing = NstIsPlaying();
	if (playing) {
		wasplaying = 1;
		schedule_stop = 1;
	}
}

GtkWidget* create_mainwindow (int xres, int yres) {

	GtkWidget *box;

	GtkWidget *menubar;
  
	GtkWidget *filemenu;
	GtkWidget *file;
	GtkWidget *open;
	GtkWidget *sep1;
	GtkWidget *quit;
	
	GtkWidget *emulatormenu;
	GtkWidget *emulator;
	GtkWidget *cont;
	GtkWidget *pause;
	GtkWidget *resetsoft;
	GtkWidget *resethard;
	GtkWidget *sep2;
	GtkWidget *fullscreen;
	GtkWidget *sep3;
	GtkWidget *loadstate;
	GtkWidget *savestate;
	GtkWidget *sep4;
	GtkWidget *flipdisk;
	GtkWidget *switchdisk;
	GtkWidget *sep5;
	GtkWidget *movieload;
	GtkWidget *movierecord;
	GtkWidget *moviestop;
	GtkWidget *sep6;
	GtkWidget *cheats;
	GtkWidget *sep7;
	GtkWidget *configuration;

	GtkWidget *helpmenu;
	GtkWidget *help;
	GtkWidget *about;
	
	GtkAccelGroup *accelgroup = gtk_accel_group_new();
	
	GtkSettings *gtksettings;

	GtkWidget *window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER);
	gtk_window_set_default_size(GTK_WINDOW(window), 200, 200);
	gtk_window_set_title(GTK_WINDOW(window), "Nestopia");
	gtk_window_set_resizable(GTK_WINDOW(window), FALSE);

	box = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	gtk_container_add(GTK_CONTAINER(window), box);

	menubar = gtk_menu_bar_new();

	filemenu = gtk_menu_new();
	emulatormenu = gtk_menu_new();
	helpmenu = gtk_menu_new();
	
	gtk_window_add_accel_group(GTK_WINDOW(window), accelgroup);

	file = gtk_menu_item_new_with_mnemonic("_File");
	open = gtk_image_menu_item_new_from_stock(GTK_STOCK_OPEN, NULL);
	sep1 = gtk_separator_menu_item_new();
	quit = gtk_image_menu_item_new_from_stock(GTK_STOCK_QUIT, NULL);
	
	emulator = gtk_menu_item_new_with_mnemonic("_Emulator");
	cont = gtk_image_menu_item_new_with_mnemonic("C_ontinue");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cont), gtk_image_new_from_stock(GTK_STOCK_MEDIA_PLAY, GTK_ICON_SIZE_MENU));
	pause = gtk_image_menu_item_new_from_stock(GTK_STOCK_MEDIA_PAUSE, NULL);
	resetsoft = gtk_image_menu_item_new_with_mnemonic("_Reset (Soft)");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(resetsoft), gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
	resethard = gtk_image_menu_item_new_with_mnemonic("Reset (_Hard)");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(resethard), gtk_image_new_from_stock(GTK_STOCK_REFRESH, GTK_ICON_SIZE_MENU));
	sep2 = gtk_separator_menu_item_new();
	fullscreen = gtk_image_menu_item_new_from_stock(GTK_STOCK_FULLSCREEN, NULL);
	sep3 = gtk_separator_menu_item_new();
	loadstate = gtk_image_menu_item_new_with_mnemonic("_Load State...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(loadstate), gtk_image_new_from_stock(GTK_STOCK_GO_BACK, GTK_ICON_SIZE_MENU));
	savestate = gtk_image_menu_item_new_with_mnemonic("_Save State...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(savestate), gtk_image_new_from_stock(GTK_STOCK_SAVE, GTK_ICON_SIZE_MENU));
	sep4 = gtk_separator_menu_item_new();
	flipdisk = gtk_image_menu_item_new_with_mnemonic("Flip FDS _Disk");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(flipdisk), gtk_image_new_from_stock(GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
	switchdisk = gtk_image_menu_item_new_with_mnemonic("_Switch FDS Disk");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(switchdisk), gtk_image_new_from_stock(GTK_STOCK_FLOPPY, GTK_ICON_SIZE_MENU));
	sep5 = gtk_separator_menu_item_new();
	movieload = gtk_image_menu_item_new_with_label("Load Movie...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(movieload), gtk_image_new_from_stock(GTK_STOCK_OPEN, GTK_ICON_SIZE_MENU));
	movierecord = gtk_image_menu_item_new_with_label("Record Movie...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(movierecord), gtk_image_new_from_stock(GTK_STOCK_MEDIA_RECORD, GTK_ICON_SIZE_MENU));
	moviestop = gtk_image_menu_item_new_with_label("Stop Movie");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(moviestop), gtk_image_new_from_stock(GTK_STOCK_MEDIA_STOP, GTK_ICON_SIZE_MENU));
	sep6 = gtk_separator_menu_item_new();
	cheats = gtk_image_menu_item_new_with_mnemonic("Ch_eats...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(cheats), gtk_image_new_from_stock(GTK_STOCK_DIALOG_WARNING, GTK_ICON_SIZE_MENU));
	sep7 = gtk_separator_menu_item_new();
	configuration = gtk_image_menu_item_new_with_mnemonic("_Configuration...");
	gtk_image_menu_item_set_image(GTK_IMAGE_MENU_ITEM(configuration), gtk_image_new_from_stock(GTK_STOCK_PREFERENCES, GTK_ICON_SIZE_MENU));

	help = gtk_menu_item_new_with_mnemonic("_Help");
	about = gtk_image_menu_item_new_from_stock(GTK_STOCK_ABOUT, NULL);

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(file), filemenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), open);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep1);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), loadstate);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), savestate);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep2);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), movieload);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), movierecord);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), moviestop);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), sep3);
	gtk_menu_shell_append(GTK_MENU_SHELL(filemenu), quit);
	
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(emulator), emulatormenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cont);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), pause);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), resetsoft);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), resethard);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep4);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), fullscreen);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep5);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), flipdisk);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), switchdisk);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep6);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), cheats);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), sep7);
	gtk_menu_shell_append(GTK_MENU_SHELL(emulatormenu), configuration);
  
	gtk_menu_item_set_submenu(GTK_MENU_ITEM(help), helpmenu);
	gtk_menu_shell_append(GTK_MENU_SHELL(helpmenu), about);
  
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), file);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), emulator);
	gtk_menu_shell_append(GTK_MENU_SHELL(menubar), help);
	
	drawingarea = gtk_drawing_area_new();
	gtk_widget_set_size_request(drawingarea, xres, yres);
	
	statusbar = gtk_statusbar_new();
	
	gtk_box_pack_start(GTK_BOX(box), menubar, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(box), drawingarea, TRUE, TRUE, 0);
	//gtk_box_pack_start(GTK_BOX(box), statusbar, FALSE, FALSE, 0);
	
	GtkTargetEntry target_entry[1];

	target_entry[0].target = (gchar *)"text/uri-list";
	target_entry[0].flags = 0;
	target_entry[0].info = 0;
	
	gtk_drag_dest_set(drawingarea, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP), 
		target_entry, sizeof(target_entry) / sizeof(GtkTargetEntry), (GdkDragAction)(GDK_ACTION_MOVE | GDK_ACTION_COPY));

	g_signal_connect(G_OBJECT(drawingarea), "drag-data-received",
		G_CALLBACK(drag_data_received), NULL);

	g_signal_connect_swapped(G_OBJECT(window), "destroy",
		G_CALLBACK(on_mainwindow_destroy), NULL);

	g_signal_connect(G_OBJECT(open), "activate",
		G_CALLBACK(on_open_clicked), NULL);

	g_signal_connect(G_OBJECT(quit), "activate",
		G_CALLBACK(on_mainwindow_destroy), NULL);

	g_signal_connect(G_OBJECT(cont), "activate",
		G_CALLBACK(on_playbutton_clicked), NULL);

	g_signal_connect(G_OBJECT(pause), "activate",
		G_CALLBACK(pause_clicked), NULL);

	g_signal_connect(G_OBJECT(resetsoft), "activate",
		G_CALLBACK(NstSoftReset), NULL);

	g_signal_connect(G_OBJECT(resethard), "activate",
		G_CALLBACK(NstHardReset), NULL);

	g_signal_connect(G_OBJECT(fullscreen), "activate",
		G_CALLBACK(video_toggle_fullscreen), NULL);

	g_signal_connect(G_OBJECT(loadstate), "activate",
		G_CALLBACK(fileio_do_state_load), NULL);

	g_signal_connect(G_OBJECT(savestate), "activate",
		G_CALLBACK(fileio_do_state_save), NULL);
		
	g_signal_connect(G_OBJECT(flipdisk), "activate",
		G_CALLBACK(FlipFDSDisk), NULL);
	
	g_signal_connect(G_OBJECT(switchdisk), "activate",
		G_CALLBACK(SwitchFDSDisk), NULL);
		
	g_signal_connect(G_OBJECT(movieload), "activate",
		G_CALLBACK(fileio_do_movie_load), NULL);
		
	g_signal_connect(G_OBJECT(movierecord), "activate",
		G_CALLBACK(fileio_do_movie_save), NULL);
		
	g_signal_connect(G_OBJECT(moviestop), "activate",
		G_CALLBACK(fileio_do_movie_stop), NULL);

	g_signal_connect(G_OBJECT(cheats), "activate",
		G_CALLBACK(on_cheatbutton_pressed), NULL);

	g_signal_connect(G_OBJECT(configuration), "activate",
		G_CALLBACK(create_config), NULL);

	g_signal_connect(G_OBJECT(about), "activate",
		G_CALLBACK(create_about), NULL);

	g_signal_connect(G_OBJECT(window), "key_press_event",
		G_CALLBACK(convert_keypress), NULL);

	g_signal_connect(G_OBJECT(window), "key_release_event",
		G_CALLBACK(convert_keypress), NULL);

	gtksettings = gtk_settings_get_default();
	g_object_set(G_OBJECT(gtksettings), "gtk-application-prefer-dark-theme", TRUE, NULL);

	gtk_widget_show_all(window);

	/*char SDL_windowhack[24];
	snprintf(SDL_windowhack, sizeof(SDL_windowhack), "SDL_WINDOWID=%ld", GDK_WINDOW_XID(gtk_widget_get_window(drawingarea)));
	set_window_id(SDL_windowhack);*/
	
	gtk_widget_modify_bg(drawingarea, GTK_STATE_NORMAL, &bg);

	return window;
}

void redraw_drawingarea(int xres, int yres) {
	gtk_widget_set_size_request(drawingarea, xres, yres);
	gtk_widget_modify_bg(drawingarea, GTK_STATE_NORMAL, &bg);
}

GtkWidget* create_config(void) {

	// Pause if playing
	bool playing = NstIsPlaying();
	if (playing) {
		schedule_stop = 1;
		wasplaying = 1;
	}
	else {	// Set it back to 0 in case the game was paused and the config is opened again
		wasplaying = 0;
	}
	
	configwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(configwindow), "Configuration");
	
	GtkWidget *bigbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *smallbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
	GtkWidget *notebook = gtk_notebook_new();

	gtk_container_add(GTK_CONTAINER(configwindow), bigbox);

	// The Video stuff
	GtkWidget *videobox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	GtkWidget *renderlabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(renderlabel), "<b>Renderer</b>");

    GtkWidget *rendercombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rendercombo), "Software");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rendercombo), "Hardware - OpenGL");
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rendercombo), "Hardware - OpenGL (bilinear)");
    gtk_combo_box_set_active(GTK_COMBO_BOX(rendercombo), conf->video_renderer);
    
    GtkWidget *filtersettingslabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(filtersettingslabel), "<b>Filter/Scaler Options</b>");
    
	GtkWidget *scalebox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *scalelabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "Scaler:", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	GtkWidget *scalecombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "None");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "NTSC");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "xBR");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "HqX");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "2xSaI");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "ScaleX");
	GtkWidget *scaleamtcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_set_active(GTK_COMBO_BOX(scalecombo), conf->video_filter);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "1x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "2x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "3x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "4x");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scaleamtcombo), conf->video_scale_factor - 1);
	gtk_box_pack_start(GTK_BOX(scalebox), scalelabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(scalebox), scalecombo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(scalebox), scaleamtcombo, FALSE, FALSE, 0);
	
	GtkWidget *ntscbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *ntsclabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "NTSC Type:", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	GtkWidget *ntsccombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(ntsccombo), "Composite");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(ntsccombo), "S-Video");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(ntsccombo), "RGB");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ntsccombo), conf->video_ntsc_mode);
	gtk_box_pack_start(GTK_BOX(ntscbox), ntsclabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(ntscbox), ntsccombo, FALSE, FALSE, 0);
	
	GtkWidget *xbrbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *xbrlabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "xBR Corner Rounding:", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	GtkWidget *xbrcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(xbrcombo), "None");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(xbrcombo), "Some");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(xbrcombo), "All");
	gtk_combo_box_set_active(GTK_COMBO_BOX(xbrcombo), conf->video_xbr_corner_rounding);
	gtk_box_pack_start(GTK_BOX(xbrbox), xbrlabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(xbrbox), xbrcombo, FALSE, FALSE, 0);
	
	GtkWidget *check_blendpix = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "xBR Pixel Blending", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_blendpix), conf->video_xbr_pixel_blending);
	
	GtkWidget *misclabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(misclabel), "<b>Misc Options</b>");
	
	GtkWidget *check_tvaspect = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "TV Aspect Ratio", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_tvaspect), conf->video_tv_aspect);
	
	GtkWidget *check_oscanmask = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Mask Overscan", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_oscanmask), conf->video_mask_overscan);
	
	GtkWidget *check_fullscreen = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Fullscreen", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fullscreen), conf->video_fullscreen);
	
	GtkWidget *check_stretchfullscreen = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Stretch when Fullscreen", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_stretchfullscreen), conf->video_preserve_aspect);

	GtkWidget *unlimitsprcheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Unlimited Sprites", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(unlimitsprcheck), conf->video_unlimited_sprites);

	gtk_box_pack_start(GTK_BOX(videobox), renderlabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), rendercombo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), filtersettingslabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), scalebox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), ntscbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), xbrbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), check_blendpix, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), misclabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), check_tvaspect, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), check_oscanmask, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), check_fullscreen, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), check_stretchfullscreen, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), unlimitsprcheck, FALSE, FALSE, 0);
	//End of the Video stuff

    // The Audio stuff
    GtkWidget *audiobox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    #ifdef OSS_ALSA    
    GtkWidget *sndapilabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(sndapilabel), "<b>Sound API</b>");
	
    GtkWidget *sndapicombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "SDL");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "ALSA");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "OSS");
	gtk_combo_box_set_active(GTK_COMBO_BOX(sndapicombo), conf->audio_api);
	#endif
	
	GtkWidget *ratelabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(ratelabel), "<b>Sample Rate</b>");
	
	GtkWidget *ratecombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "11025 Hz");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "22050 Hz");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "44100 Hz");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "48000 Hz");
	// This is ugly and needs to be rethought.
	switch (conf->audio_sample_rate) {
		case 11025:
			gtk_combo_box_set_active(GTK_COMBO_BOX(ratecombo), 0);
			break;
		case 22050:
			gtk_combo_box_set_active(GTK_COMBO_BOX(ratecombo), 1);
			break;
		case 44100:
			gtk_combo_box_set_active(GTK_COMBO_BOX(ratecombo), 2);
			break;
		case 48000:
			gtk_combo_box_set_active(GTK_COMBO_BOX(ratecombo), 3);
			break;
		default: break;
	}
	
	GtkWidget *audsettingslabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(audsettingslabel), "<b>Settings</b>");

	GtkWidget *volumebox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *volumelabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "Volume", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	GtkWidget *volumescroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 5, 0)));
	gtk_widget_set_size_request(volumescroll, 128, 24);
	gtk_range_set_value(GTK_RANGE(volumescroll), conf->audio_volume);
	gtk_box_pack_start(GTK_BOX(volumebox), volumelabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(volumebox), volumescroll, FALSE, FALSE, 0);

	GtkWidget *surrbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *surrcheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Lite Surround", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(surrcheck), conf->audio_surround);
	GtkWidget *surrscroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(gtk_adjustment_new (0, 0, 100, 1, 5, 0)));
	gtk_widget_set_size_request(surrscroll, 128, 24);
	gtk_range_set_value(GTK_RANGE(surrscroll), conf->audio_surround_multiplier);
	gtk_box_pack_start(GTK_BOX(surrbox), surrcheck, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(surrbox), surrscroll, FALSE, FALSE, 0);

	GtkWidget *stereocheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Stereo", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stereocheck), conf->audio_stereo);

	GtkWidget *excitecheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Stereo Exciter", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(excitecheck), conf->audio_stereo_exciter);
	
	#ifdef OSS_ALSA
	gtk_box_pack_start(GTK_BOX(audiobox), sndapilabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), sndapicombo, FALSE, FALSE, 0);
	#endif
	gtk_box_pack_start(GTK_BOX(audiobox), ratelabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), ratecombo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), audsettingslabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), volumebox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), surrbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), stereocheck, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), excitecheck, FALSE, FALSE, 0);
	// End of the Audio stuff
    
    // The Input stuff
	GtkWidget *inputbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);

	char svgpath[1024];
	snprintf(svgpath, sizeof(svgpath), "%s/icons/nespad.svg", DATADIR);
	
	// Load the NES pad svg from local source dir if make install hasn't been done
	struct stat svgstat;
	if (stat(svgpath, &svgstat) == -1) {
		snprintf(svgpath, sizeof(svgpath), "source/unix/icons/nespad.svg");
	}
	
	GtkWidget *inputgamepadbox = gtk_widget_new(GTK_TYPE_BOX, "halign", GTK_ALIGN_START, "margin", 10, NULL);

	GtkWidget *nespad = gtk_widget_new(GTK_TYPE_IMAGE, "halign", GTK_ALIGN_CENTER, "expand", TRUE, "file", svgpath, "margin", 10, NULL);
	//gtk_widget_set_size_request(nespad, 300, 400);
	
	GtkWidget *inputseparator1 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);
	
	GtkWidget *playerselectcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-top", 5, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (playerselectcombo), "Player 1");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (playerselectcombo), "Player 2");
	gtk_combo_box_set_active(GTK_COMBO_BOX(playerselectcombo), 0);
	
	GtkWidget *upbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "U", "halign", GTK_ALIGN_START, "margin-right", 4, "margin-left", 6, NULL);
	GtkWidget *downbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "D", "halign", GTK_ALIGN_START, "margin-right", 4, NULL);
	GtkWidget *leftbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "L", "halign", GTK_ALIGN_START, "margin-right", 4, NULL);
	GtkWidget *rightbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "R", "halign", GTK_ALIGN_START, "margin-right", 16, NULL);
	GtkWidget *selectbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "SELECT", "halign", GTK_ALIGN_START, "margin-right", 4,NULL);
	GtkWidget *startbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "START", "halign", GTK_ALIGN_START, "margin-right", 16,NULL);
	GtkWidget *bbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "B", "halign", GTK_ALIGN_START, "margin-right", 4, NULL);
	GtkWidget *abutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", "A", "halign", GTK_ALIGN_START, NULL);

	
	GtkWidget *inputseparator2 = gtk_separator_new(GTK_ORIENTATION_HORIZONTAL);

	GtkWidget *inputmetabox = gtk_widget_new(GTK_TYPE_BOX, "halign", GTK_ALIGN_START, "margin", 10, NULL);

	GtkWidget *rewindstart = gtk_widget_new(GTK_TYPE_BUTTON, "label", "Rewind Start", "halign", GTK_ALIGN_START, "margin-right", 4, NULL);
	GtkWidget *rewindstop = gtk_widget_new(GTK_TYPE_BUTTON, "label", "Rewind Stop", "halign", GTK_ALIGN_START, NULL);

	gtk_box_pack_start(GTK_BOX(inputbox), nespad, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(inputbox), inputseparator1, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputbox), playerselectcombo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputbox), inputgamepadbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputbox), inputseparator2, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputbox), inputmetabox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), upbutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), downbutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), leftbutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), rightbutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), selectbutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), startbutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), bbutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputgamepadbox), abutton, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputmetabox), rewindstart, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(inputmetabox), rewindstop, FALSE, FALSE, 0);
    
    // The Misc stuff
    GtkWidget *miscbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
	GtkWidget *videolabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(videolabel), "<b>Video Region</b>");
    GtkWidget *videocombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(videocombo), "Auto");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(videocombo), "NTSC");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(videocombo), "PAL");
	gtk_combo_box_set_active(GTK_COMBO_BOX(videocombo), conf->misc_video_region);
    
    GtkWidget *systemlabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(systemlabel), "<b>Default System</b>");
	GtkWidget *systemcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "NES (NTSC)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "NES (PAL)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "Famicom");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "Dendy");
	gtk_combo_box_set_active(GTK_COMBO_BOX(systemcombo), conf->misc_default_system);

	GtkWidget *spatchlabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(spatchlabel), "<b>Soft Patching</b>");
	GtkWidget *spatchcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(spatchcombo), "Off");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(spatchcombo), "On");
	gtk_combo_box_set_active(GTK_COMBO_BOX(spatchcombo), conf->misc_soft_patching);

	gtk_box_pack_start(GTK_BOX(miscbox), videolabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(miscbox), videocombo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(miscbox), systemlabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(miscbox), systemcombo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(miscbox), spatchlabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(miscbox), spatchcombo, FALSE, FALSE, 0);

	//Structuring the notebook
	GtkWidget *labelvideo = gtk_label_new("Video");
	GtkWidget *labelaudio = gtk_label_new("Audio");
	GtkWidget *labelinput = gtk_label_new("Input");
	GtkWidget *labelmisc = gtk_label_new("Misc");
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), videobox, labelvideo);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), audiobox, labelaudio);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), inputbox, labelinput);
	gtk_notebook_append_page(GTK_NOTEBOOK(notebook), miscbox, labelmisc);

	// The OK button stuff
	GtkWidget *okbutton = gtk_widget_new(GTK_TYPE_BUTTON, "label", GTK_STOCK_OK, "halign", GTK_ALIGN_END, "margin-top", 8, "margin-bottom", 8, "margin-right", 8, NULL);
	gtk_button_set_use_stock(GTK_BUTTON(okbutton), TRUE);

	//Structuring the window
	gtk_box_pack_start(GTK_BOX(bigbox), notebook, TRUE, TRUE, 0);
	gtk_box_pack_start(GTK_BOX(bigbox), smallbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(smallbox), okbutton, FALSE, FALSE, 0);
	
	//Set the icon
	GdkPixbuf *app_icon = get_icon();
	gtk_window_set_icon(GTK_WINDOW(configwindow), app_icon);
	
	//Config
	g_signal_connect(G_OBJECT(okbutton), "clicked",
		G_CALLBACK(okbutton_clicked), NULL);

	g_signal_connect(G_OBJECT(configwindow), "destroy",
		G_CALLBACK(configwindow_destroyed), NULL);
	
	//Video
	g_signal_connect(G_OBJECT(scaleamtcombo), "changed",
		G_CALLBACK(on_scaleamtcombo_changed), NULL);

	g_signal_connect(G_OBJECT(scalecombo), "changed",
		G_CALLBACK(on_scalecombo_changed), NULL);

	g_signal_connect(G_OBJECT(check_blendpix), "toggled",
		G_CALLBACK(on_check_blendpix_toggled), NULL);

	g_signal_connect(G_OBJECT(check_tvaspect), "toggled",
		G_CALLBACK(on_check_tvaspect_toggled), NULL);
		
	g_signal_connect(G_OBJECT(check_oscanmask), "toggled",
		G_CALLBACK(on_check_oscanmask_toggled), NULL);
	
	g_signal_connect(G_OBJECT(check_fullscreen), "toggled",
		G_CALLBACK(on_check_fullscreen_toggled), NULL);
		
	g_signal_connect(G_OBJECT(check_stretchfullscreen), "toggled",
		G_CALLBACK(on_check_fsnativeres_toggled), NULL);

	g_signal_connect(G_OBJECT(unlimitsprcheck), "toggled",
		G_CALLBACK(on_unlimitsprcheck_toggled), NULL);

	g_signal_connect(G_OBJECT(videocombo), "changed",
		G_CALLBACK(on_videocombo_changed), NULL);

	g_signal_connect(G_OBJECT(ntsccombo), "changed",
		G_CALLBACK(on_ntsccombo_changed), NULL);

	g_signal_connect(G_OBJECT(rendercombo), "changed",
		G_CALLBACK(on_rendercombo_changed), NULL);
		
	g_signal_connect(G_OBJECT(xbrcombo), "changed",
		G_CALLBACK(on_xbrcombo_changed), NULL);

	//Audio
	g_signal_connect(G_OBJECT(volumescroll), "value_changed",
		G_CALLBACK(on_volumescroll_value_changed), NULL);

	g_signal_connect(G_OBJECT(volumescroll), "configure_event",
		G_CALLBACK(on_volumescroll_configure_event), NULL);

	g_signal_connect(G_OBJECT(surrscroll), "value_changed",
		G_CALLBACK(on_surrscroll_value_changed), NULL);

	g_signal_connect(G_OBJECT(surrcheck), "toggled",
		G_CALLBACK(on_surrcheck_toggled), NULL);

	g_signal_connect(G_OBJECT(excitecheck), "toggled",
		G_CALLBACK(on_excitecheck_toggled), NULL);

	g_signal_connect(G_OBJECT(stereocheck), "toggled",
		G_CALLBACK(on_stereocheck_toggled), NULL);

	g_signal_connect(G_OBJECT(ratecombo), "changed",
		G_CALLBACK(on_ratecombo_changed), NULL);

	g_signal_connect(G_OBJECT(ratecombo), "configure_event",
		G_CALLBACK(on_ratecombo_configure_event), NULL);
	#ifdef OSS_ALSA
	g_signal_connect(G_OBJECT(sndapicombo), "changed",
		G_CALLBACK(on_sndapicombo_changed), NULL);
	#endif

	//Input
	g_signal_connect(G_OBJECT(playerselectcombo), "changed",
		G_CALLBACK(playercombo_changed), NULL);

	g_signal_connect(G_OBJECT(upbutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(0));

	g_signal_connect(G_OBJECT(downbutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(1));

	g_signal_connect(G_OBJECT(leftbutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(2));

	g_signal_connect(G_OBJECT(rightbutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(3));

	g_signal_connect(G_OBJECT(selectbutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(7));

	g_signal_connect(G_OBJECT(startbutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(6));

	g_signal_connect(G_OBJECT(abutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(4));

	g_signal_connect(G_OBJECT(bbutton), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(5));
		
	g_signal_connect(G_OBJECT(rewindstart), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(26));
		
	g_signal_connect(G_OBJECT(rewindstop), "clicked",
		G_CALLBACK(inputcfg_clicked), gpointer(27));

	//Misc
	g_signal_connect(G_OBJECT(systemcombo), "changed",
		G_CALLBACK(on_systemcombo_changed), NULL);

	g_signal_connect(G_OBJECT(spatchcombo), "changed",
		G_CALLBACK(on_spatchcombo_changed), NULL);

	gtk_widget_show_all(configwindow);

	return configwindow;
}

/*GtkWidget* create_nsfplayer (void) {

	// Pause if playing
	bool playing = NstIsPlaying();
	if (playing) {
		schedule_stop = 1;
		wasplaying = 1;
	}
	else {	// Set it back to 0 in case the game was paused and the config is opened again
		wasplaying = 0;
	}

	//GtkWidget *nsfplayer;
	GtkWidget *nsffixed;

	GtkAdjustment *nsfspinbutton_adj;
	GtkWidget *nsfspinbutton;
	GtkWidget *nsfstop;
	GtkWidget *nsfplay;

	nsfplayer = gtk_window_new (GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title (GTK_WINDOW (nsfplayer), "NSF Player");

	nsffixed = gtk_fixed_new();
	gtk_container_add (GTK_CONTAINER (nsfplayer), nsffixed);

	nsfmaker = gtk_label_new ("");
	gtk_widget_show (nsfmaker);
	gtk_fixed_put (GTK_FIXED (nsffixed), nsfmaker, 8, 64);
	gtk_widget_set_size_request (nsfmaker, 320, 24);

	nsfauthor = gtk_label_new ("");
	gtk_widget_show (nsfauthor);
	gtk_fixed_put (GTK_FIXED (nsffixed), nsfauthor, 8, 32);
	gtk_widget_set_size_request (nsfauthor, 320, 24);
	gtk_label_set_justify (GTK_LABEL (nsfauthor), GTK_JUSTIFY_CENTER);

	nsftitle = gtk_label_new ("");
	gtk_widget_show (nsftitle);
	gtk_fixed_put (GTK_FIXED (nsffixed), nsftitle, 8, 0);
	gtk_widget_set_size_request (nsftitle, 320, 24);
	gtk_label_set_justify (GTK_LABEL (nsftitle), GTK_JUSTIFY_CENTER);

	nsfstop = gtk_button_new_from_stock ("gtk-media-stop");
	gtk_fixed_put (GTK_FIXED (nsffixed), nsfstop, 56, 104);
	gtk_widget_set_size_request (nsfstop, 96, 32);

	nsfplay = gtk_button_new_from_stock ("gtk-media-play");
	gtk_fixed_put (GTK_FIXED (nsffixed), nsfplay, 160, 104);
	gtk_widget_set_size_request (nsfplay, 88, 32);
	
	nsfspinbutton_adj = gtk_adjustment_new (1, 0, 100, 1, 10, 10);
	nsfspinbutton = gtk_spin_button_new (GTK_ADJUSTMENT (nsfspinbutton_adj), 1, 0);
	gtk_fixed_put (GTK_FIXED (nsffixed), nsfspinbutton, 272, 104);
	gtk_widget_set_size_request (nsfspinbutton, 72, 35);
	
	g_signal_connect (G_OBJECT(nsfspinbutton), "change_value",
		G_CALLBACK (on_nsfspinbutton_change_value), NULL);

	g_signal_connect (G_OBJECT(nsfspinbutton), "value_changed",
		G_CALLBACK (on_nsfspinbutton_value_changed), NULL);

	g_signal_connect (G_OBJECT(nsfstop), "clicked",
		G_CALLBACK (on_nsfstop_clicked), NULL);

	g_signal_connect (G_OBJECT(nsfplay), "clicked",
		G_CALLBACK (on_nsfplay_clicked), NULL);

	g_signal_connect (G_OBJECT(nsfplayer), "destroy",
		G_CALLBACK (on_nsfplayer_destroy), NULL);

	gtk_widget_show_all(nsfplayer);

	return nsfplayer;
}*/

GtkWidget* create_about (void) {

	char svgpath[1024];
	snprintf(svgpath, sizeof(svgpath), "%s/icons/nestopia.svg", DATADIR);
	
	// Load the SVG from local source dir if make install hasn't been done
	struct stat svgstat;
	if (stat(svgpath, &svgstat) == -1) {
		snprintf(svgpath, sizeof(svgpath), "source/unix/icons/nestopia.svg");
	}
	
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(svgpath, 192, 192, NULL);
	
	GtkWidget *aboutdialog = gtk_about_dialog_new();
	
	GdkPixbuf *app_icon = get_icon();
	gtk_window_set_icon(GTK_WINDOW(aboutdialog), app_icon);
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutdialog), pixbuf);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutdialog), "Nestopia UE");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), VERSION);
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutdialog), "An accurate Nintendo Entertainment System Emulator");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutdialog), "http://0ldsk00l.ca/nestopia");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutdialog), "(c) 2012-2013, R. Danbrook\n(c) 2007-2008, R. Belmont\n(c) 2003-2008, Martin Freij\n\nIcon based on art from Trollekop");
	g_object_unref(pixbuf), pixbuf = NULL;
	gtk_dialog_run(GTK_DIALOG(aboutdialog));
	gtk_widget_destroy(aboutdialog);
	
	return aboutdialog;
}

void create_messagewindow(char* message) {    
	messagewindow = gtk_message_dialog_new(GTK_WINDOW(mainwindow), GTK_DIALOG_DESTROY_WITH_PARENT, GTK_MESSAGE_INFO, GTK_BUTTONS_OK, message);
	gtk_dialog_run (GTK_DIALOG (messagewindow));
	gtk_widget_destroy (messagewindow);
}

// Adapted the following from FCEUX and Gens/GS, whoever wrote it first is unknown

unsigned int translate_gdk_sdl(int gdk_keyval) {
	
	if (!(gdk_keyval & 0xFF00))	{

		//printf("GDK key: %x\n", gdk_keyval);
		gdk_keyval = tolower(gdk_keyval);
		
		return gdk_keyval;
	}
	
	if (gdk_keyval & 0xFFFF0000) {

		printf("Unhandled extended key: 0x%08X\n", gdk_keyval);

		return 0;
	}
	
	// Non-ASCII symbol.
	static const SDL_Keycode gdk_to_sdl_table[0x100] =
	{
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
	
	//printf("GDK key: %x\n", gdk_keyval);
	//printf("SDL key: %s\n", SDL_GetKeyName(sdl_keycode));
	
	return sdl_keycode;
}

int convert_keypress(GtkWidget *grab, GdkEventKey *event, gpointer user_data) {

	SDL_Event sdlevent;
	SDL_Keycode sdlkeycode;
	char keystate;
	
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
			fprintf(stderr, "Unhandled GDK event type: %d\n", event->type);
			return FALSE;
	}
	
	sdlkeycode = (SDL_Keycode)translate_gdk_sdl(event->keyval);
	
	sdlevent.key.keysym.sym = sdlkeycode;
	
	if (sdlkeycode != 0) {
		SDL_PushEvent(&sdlevent);
		
		const Uint8 *statebuffer = SDL_GetKeyboardState(NULL);
		Uint8 *state = (Uint8*)statebuffer;
		state[SDL_GetScancodeFromKey(sdlkeycode)] = keystate;
	}

	return FALSE;
}

void set_window_id(char* sdlwindowid) {
	//printf("%s\n", sdlwindowid);
	snprintf(windowid, sizeof(windowid), "%s", sdlwindowid);
}
