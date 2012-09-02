/*
	NEStopia / Linux
	Port by R. Belmont
	
	uihelp.cpp - UI callback functions
*/

#include <stdlib.h>
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
#include "oss.h"
#include "settings.h"
#include "auxio.h"
#include "input.h"
#include "controlconfig.h"
#include "cheats.h"
#include "seffect.h"
#include "main.h"
#include "GL/glu.h"

extern "C" {
#include <gtk/gtk.h>

#include "interface.h"
#include "support.h"
#include "callbacks.h"

// get icon data
#include "nsticon.c"
}

#define DRAG_TAR_NAME_0		"text/uri-list"
#define DRAG_TAR_INFO_0		0

using namespace Nes::Api;
using namespace LinuxNst;

extern Emulator emulator;

static Settings *sSettings;
static CheatMgr *sCheatMgr;

static GdkPixbuf *app_icon;

GtkWidget *mainwindow;
static GtkWidget *check_fullscreen, *check_unlimitspr, *check_controls, *check_alsa, *check_stereo, *check_exciter, *check_surround;
static GtkWidget *combo_ntsc, *combo_rate, *combo_scale, *combo_videomode, *button_play, *button_nsfplay, *button_nsfstop, *spin_nsf;
static GtkWidget *text_nsftitle, *text_nsfauthor, *text_nsfmaker, *text_volume, *scroll_volume, *notebook_main;
static GtkWidget *combo_render, *combo_favor, *combo_scaleamt, *combo_config, *combo_spatch, *scroll_surround, *text_surround;
static GtkWidget *combo_sndapi;

static char volumestr[5], surrmulstr[5];

void on_nsfspinbutton_input(GtkSpinButton   *spinbutton, GtkScrollType    scroll, gpointer         user_data)
{
}

void on_nsfspinbutton_change_value(GtkSpinButton   *spinbutton, GtkScrollType scroll, gpointer user_data)
{
}

void on_nsfspinbutton_value_changed(GtkSpinButton   *spinbutton, gpointer         user_data)
{
	Nsf nsf( emulator );

	nsf.SelectSong((int)gtk_spin_button_get_value(spinbutton));
}

void on_nsfplay_clicked(GtkButton       *button,  gpointer         user_data)
{
	NstPlayNsf();

	gtk_widget_set_sensitive(button_nsfplay, FALSE);
	gtk_widget_set_sensitive(button_nsfstop, TRUE);
}

void on_nsfstop_clicked(GtkButton       *button, gpointer         user_data)
{
	NstStopNsf();

	gtk_widget_set_sensitive(button_nsfplay, TRUE);
	gtk_widget_set_sensitive(button_nsfstop, FALSE);
}

void on_videocombo_changed(GtkComboBox     *combobox, gpointer         user_data)
{
	 sSettings->SetVideoMode(gtk_combo_box_get_active(combobox));
}

void on_mainwindow_destroy(GtkObject *object, gpointer user_data)
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
		NstStopPlaying();
	}
	
	dialog = gtk_file_chooser_dialog_new ("Choose a game",
					      GTK_WINDOW(mainwindow),
					      GTK_FILE_CHOOSER_ACTION_OPEN,
					      GTK_STOCK_CANCEL, GTK_RESPONSE_CANCEL,
					      GTK_STOCK_OPEN, GTK_RESPONSE_ACCEPT,
					      NULL);

	filter = gtk_file_filter_new();
	gtk_file_filter_set_name(filter, "NES files + archives");
	gtk_file_filter_add_pattern(filter, "*.nes");
	gtk_file_filter_add_pattern(filter, "*.fds");
	gtk_file_filter_add_pattern(filter, "*.unf");
	gtk_file_filter_add_pattern(filter, "*.unif");
	gtk_file_filter_add_pattern(filter, "*.nsf");
	gtk_file_filter_add_pattern(filter, "*.zip");
	gtk_file_filter_add_pattern(filter, "*.7z");
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(dialog), filter);

	if (gtk_dialog_run (GTK_DIALOG (dialog)) == GTK_RESPONSE_ACCEPT)
	{
		char *filename;
		int i;

		filename = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (dialog));

		// make sure open starts in this location next time
		set_cur_path(filename);

		// load the cartridge
		NstLoadGame(filename);
		g_free (filename);
	}

	gtk_widget_destroy (dialog);
}

void on_playbutton_clicked(GtkButton *button,  gpointer user_data)
{
	NstPlayGame();
}

void
on_check_fullscreen_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	sSettings->SetFullscreen(gtk_toggle_button_get_active(togglebutton));
}

void
on_unlimitsprcheck_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	sSettings->SetSprlimit(gtk_toggle_button_get_active(togglebutton));
}

void
on_scalecombo_changed                  (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
	 sSettings->SetScale(gtk_combo_box_get_active(combobox));
}

void
on_controlcheck_toggled                (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	sSettings->SetUseJoypads(gtk_toggle_button_get_active(togglebutton));
}

void
on_stereocheck_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	Sound sound( emulator );

	sSettings->SetStereo(gtk_toggle_button_get_active(togglebutton));

	if (NstIsPlaying())
	{
		if (sSettings->GetStereo())
		{
			sound.SetSpeaker( Sound::SPEAKER_STEREO );
		}
		else
		{
			sound.SetSpeaker( Sound::SPEAKER_MONO );
		}
	}
}

void
on_surrcheck_toggled                   (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	sSettings->SetUseSurround(gtk_toggle_button_get_active(togglebutton)); 
}

void
on_excitecheck_toggled                 (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	sSettings->SetUseExciter(gtk_toggle_button_get_active(togglebutton)); 
}

void
on_ratecombo_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
	 sSettings->SetRate(gtk_combo_box_get_active(combobox));
}

gboolean
on_ratecombo_configure_event           (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
}

void
on_rendercombo_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
	 sSettings->SetRenderType(gtk_combo_box_get_active(combobox));
}

void
on_favorcombo_changed                   (GtkComboBox     *combobox,
                                        gpointer         user_data)
{
	 sSettings->SetPrefSystem(gtk_combo_box_get_active(combobox));
}

void on_scaleamtcombo_changed(GtkComboBox     *combobox, gpointer         user_data)
{
	 sSettings->SetScaleAmt(gtk_combo_box_get_active(combobox));
}


void on_configcombo_changed(GtkComboBox     *combobox, gpointer         user_data)
{
	 sSettings->SetConfigItem(gtk_combo_box_get_active(combobox));
}

void on_spatchcombo_changed(GtkComboBox     *combobox, gpointer         user_data)
{
	 sSettings->SetSoftPatch(gtk_combo_box_get_active(combobox));
}

void on_sndapicombo_changed(GtkComboBox     *combobox, gpointer         user_data)
{
	 sSettings->SetSndAPI(gtk_combo_box_get_active(combobox));
}

void
on_volumescroll_value_changed          (GtkRange        *range,
                                        gpointer         user_data)
{
	Sound sound( emulator );

	sSettings->SetVolume((int)gtk_range_get_value(range));
	sprintf(volumestr, "%d", sSettings->GetVolume());
	gtk_label_set_text(GTK_LABEL(text_volume), volumestr);

	if (NstIsPlaying())
	{
		sound.SetVolume(Sound::ALL_CHANNELS, sSettings->GetVolume());
	}
}

void on_surrscroll_value_changed(GtkRange *range, gpointer user_data)
{
	sSettings->SetSurrMult((int)gtk_range_get_value(range));
	sprintf(surrmulstr, "%d", sSettings->GetSurrMult());
	gtk_label_set_text(GTK_LABEL(text_surround), surrmulstr);
}

void on_cheatbutton_pressed(GtkButton *button, gpointer user_data)
{
	sCheatMgr->ShowManager();
}

gboolean
on_volumescroll_configure_event        (GtkWidget       *widget,
                                        GdkEventConfigure *event,
                                        gpointer         user_data)
{
}

void on_ntsccombo_changed (GtkComboBox *combobox, gpointer user_data)
{
	sSettings->SetNtscMode(gtk_combo_box_get_active(combobox));
}

void on_configbutton_clicked(GtkButton *button, gpointer user_data)
{
	NstLaunchConfig();
}

static void load_file_by_uri(char *filename)
{
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
}

static void ui_drag_data_recieved(GtkWidget *widget, GdkDragContext *dc, gint x, gint y, GtkSelectionData *selection_data, guint info, guint t, gpointer data)
{
	GtkWidget *source_widget;

        if ((widget == NULL) || (dc == NULL))
	{
                return;
	}

	if (selection_data == NULL)
	{
		return;
	}

        if (selection_data->length < 0)
	{
                return;
	}

	if (info == 0)
	{
		gchar *filename = (gchar *)selection_data->data;
		int i, datalen;
		char *root;

		// for multiple files in a drag, we get a list of file:// URIs separated by LF/CRs
		// we only accept one in NEStopia
		datalen = strlen(filename);
		root = filename;
		for (i = 0; i < datalen; i++)
		{
			if ((filename[i] == 0x0d) || (filename[i] == 0x0a))
			{
				filename[i] = '\0';

				load_file_by_uri(root);
				return;
			}
		}
	}
}

void UIHelp_Init(int argc, char *argv[], LinuxNst::Settings *settings, LinuxNst::CheatMgr *cheatmgr)
{
	GtkTargetEntry target_entry[1];

	gtk_set_locale();
	gtk_init(&argc, &argv);

	sSettings = settings;
	sCheatMgr = cheatmgr;

	// crank up our GUI
	mainwindow = create_mainwindow();

	// set up the icon
	app_icon = gdk_pixbuf_new_from_inline(-1, nsticon, FALSE, NULL);
	gtk_window_set_icon(GTK_WINDOW(mainwindow), app_icon);

	// get references to all the GUI widgets
	notebook_main = lookup_widget(mainwindow, "notebook1"); 
	check_fullscreen = lookup_widget(mainwindow, "check_fullscreen");
	check_unlimitspr = lookup_widget(mainwindow, "unlimitsprcheck"); 
	check_controls = lookup_widget(mainwindow, "controlcheck"); 
	check_stereo = lookup_widget(mainwindow, "stereocheck"); 
	check_exciter = lookup_widget(mainwindow, "excitecheck"); 
	check_surround = lookup_widget(mainwindow, "surrcheck"); 
	combo_rate = lookup_widget(mainwindow, "ratecombo");
	combo_scale = lookup_widget(mainwindow, "scalecombo");
	combo_videomode = lookup_widget(mainwindow, "videocombo"); 
	combo_ntsc = lookup_widget(mainwindow, "ntsccombo");
	combo_render = lookup_widget(mainwindow, "rendercombo");
	combo_favor = lookup_widget(mainwindow, "favorcombo");
	combo_scaleamt = lookup_widget(mainwindow, "scaleamtcombo");
	combo_config = lookup_widget(mainwindow, "configcombo");
	combo_spatch = lookup_widget(mainwindow, "spatchcombo");
	combo_sndapi = lookup_widget(mainwindow, "sndapicombo");
	button_play = lookup_widget(mainwindow, "playbutton");
	button_nsfstop = lookup_widget(mainwindow, "nsfstop"); 
	button_nsfplay = lookup_widget(mainwindow, "nsfplay"); 
	spin_nsf = lookup_widget(mainwindow, "nsfspinbutton");
	text_nsftitle = lookup_widget(mainwindow, "nsftitle"); 
	text_nsfauthor = lookup_widget(mainwindow, "nsfauthor"); 
	text_nsfmaker = lookup_widget(mainwindow, "nsfmaker"); 
	text_volume = lookup_widget(mainwindow, "volumelabel");
	text_surround = lookup_widget(mainwindow, "surroundlabel");
	scroll_volume = lookup_widget(mainwindow, "volumescroll");
	scroll_surround = lookup_widget(mainwindow, "surrscroll");

	// set them all up to reflect the saved settings
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fullscreen), sSettings->GetFullscreen());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_unlimitspr), sSettings->GetSprlimit() ^ 1);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_controls), sSettings->GetUseJoypads());
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_stereo), sSettings->GetStereo());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_rate), sSettings->GetRawRate());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_scale), sSettings->GetScale());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_videomode), sSettings->GetVideoMode());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_ntsc), sSettings->GetNtscMode());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_render), sSettings->GetRenderType());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_favor), sSettings->GetPrefSystem());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_scaleamt), sSettings->GetScaleAmt());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_config), sSettings->GetConfigItem());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_spatch), sSettings->GetSoftPatch());
	gtk_combo_box_set_active(GTK_COMBO_BOX(combo_sndapi), sSettings->GetSndAPI());
	gtk_widget_set_sensitive(button_play, FALSE);
	gtk_widget_set_sensitive(button_nsfstop, FALSE);
	gtk_widget_set_sensitive(button_nsfplay, FALSE);
	gtk_widget_set_sensitive(spin_nsf, FALSE);
	gtk_range_set_value(GTK_RANGE(scroll_volume), sSettings->GetVolume());
	gtk_range_set_value(GTK_RANGE(scroll_surround), sSettings->GetSurrMult());

	sprintf(volumestr, "%d", sSettings->GetVolume());
	gtk_label_set_text(GTK_LABEL(text_volume), volumestr);

	sprintf(surrmulstr, "%d", sSettings->GetSurrMult());
	gtk_label_set_text(GTK_LABEL(text_surround), surrmulstr);

	// set up the Open button as a drop target
	target_entry[0].target = (gchar *)DRAG_TAR_NAME_0;
	target_entry[0].flags = 0;
	target_entry[0].info = DRAG_TAR_INFO_0;

	gtk_drag_dest_set(notebook_main, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP), 
		target_entry, sizeof(target_entry) / sizeof(GtkTargetEntry), (GdkDragAction)(GDK_ACTION_MOVE | GDK_ACTION_COPY));
        gtk_signal_connect(GTK_OBJECT(notebook_main), "drag_data_received", GTK_SIGNAL_FUNC(ui_drag_data_recieved), NULL);

	// show the window
	gtk_widget_show(mainwindow);
}

void UIHelp_Unload(void)
{
	// disable the widgets since we unloaded them
	gtk_widget_set_sensitive(button_play, FALSE);
	gtk_widget_set_sensitive(button_nsfplay, FALSE);
	gtk_widget_set_sensitive(button_nsfstop, FALSE);
	gtk_widget_set_sensitive(spin_nsf, FALSE);

	// and kill the NSF text since we've unloaded too
	gtk_label_set_text(GTK_LABEL(text_nsftitle), " ");
	gtk_label_set_text(GTK_LABEL(text_nsfauthor), " ");
	gtk_label_set_text(GTK_LABEL(text_nsfmaker), " ");
}

void UIHelp_NSFLoaded(void)
{
	Nsf nsf( emulator );

	gtk_widget_set_sensitive(button_nsfplay, TRUE);
	gtk_widget_set_sensitive(spin_nsf, TRUE);

	// show the NSF info
	gtk_label_set_text(GTK_LABEL(text_nsftitle), nsf.GetName());
	gtk_label_set_text(GTK_LABEL(text_nsfauthor), nsf.GetArtist());
	gtk_label_set_text(GTK_LABEL(text_nsfmaker), nsf.GetCopyright());

	gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_nsf), nsf.GetCurrentSong());
	gtk_spin_button_set_range(GTK_SPIN_BUTTON(spin_nsf), nsf.GetStartingSong(), nsf.GetNumSongs());
}

void UIHelp_GameLoaded(void)
{
	gtk_widget_set_sensitive(button_play, TRUE);
	gtk_widget_set_sensitive(button_nsfstop, FALSE);
	gtk_widget_set_sensitive(button_nsfplay, FALSE);
	gtk_widget_set_sensitive(spin_nsf, FALSE);
}

// returns NEStopia's icon for child windows to use
GdkPixbuf *UIHelp_GetNSTIcon()
{
	return app_icon;
}
