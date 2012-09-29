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
		NstStopPlaying();
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

	gtk_widget_destroy(dialog);
	NstPlayGame();
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
	//sprintf(volumestr, "%d", sSettings->GetVolume());
	//gtk_label_set_text(GTK_LABEL(text_volume), volumestr);

	if (NstIsPlaying())
	{
		sound.SetVolume(Sound::ALL_CHANNELS, sSettings->GetVolume());
	}
}

void on_surrscroll_value_changed(GtkRange *range, gpointer user_data)
{
	sSettings->SetSurrMult((int)gtk_range_get_value(range));
	//sprintf(surrmulstr, "%d", sSettings->GetSurrMult());
	//gtk_label_set_text(GTK_LABEL(text_surround), surrmulstr);
}

void on_cheatbutton_pressed(GtkButton *button, gpointer user_data)
{
	sCheatMgr->ShowManager();
}

void on_aboutbutton_clicked(GtkButton *button,  gpointer user_data)
{
	create_about();
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

/* Does this drag'n'drop function even make sense? Maybe I'll bring this back later if I do gtkglext stuff
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
} */

void UIHelp_Init(int argc, char *argv[], LinuxNst::Settings *settings, LinuxNst::CheatMgr *cheatmgr)
{
	//GtkTargetEntry target_entry[1];

	//gtk_set_locale();
	gtk_init(&argc, &argv);

	sSettings = settings;
	sCheatMgr = cheatmgr;

	// crank up our GUI
	mainwindow = create_mainwindow();

	// set up the icon
	app_icon = gdk_pixbuf_new_from_inline(-1, nsticon, FALSE, NULL);
	gtk_window_set_icon(GTK_WINDOW(mainwindow), app_icon);

	/*// get references to all the GUI widgets
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
	gtk_label_set_text(GTK_LABEL(text_surround), surrmulstr);*/

	// set up the Open button as a drop target
	//target_entry[0].target = (gchar *)DRAG_TAR_NAME_0;
	//target_entry[0].flags = 0;
	//target_entry[0].info = DRAG_TAR_INFO_0;

	/* This is the other piece of the drag'n'drop code that I don't really care about
	gtk_drag_dest_set(notebook_main, (GtkDestDefaults)(GTK_DEST_DEFAULT_MOTION | GTK_DEST_DEFAULT_HIGHLIGHT | GTK_DEST_DEFAULT_DROP), 
		target_entry, sizeof(target_entry) / sizeof(GtkTargetEntry), (GdkDragAction)(GDK_ACTION_MOVE | GDK_ACTION_COPY));
        gtk_signal_connect(GTK_OBJECT(notebook_main), "drag_data_received", GTK_SIGNAL_FUNC(ui_drag_data_recieved), NULL); */

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

GtkWidget* create_videoconfig (void) {

	GtkWidget *videowindow;
	GtkWidget *videofixed;

	GtkWidget *scaleamtcombo;
	GtkWidget *scalelabel;
	GtkWidget *filterlabel;
	GtkWidget *scalecombo;
	GtkWidget *check_fullscreen;
	GtkWidget *unlimitsprcheck;
	GtkWidget *videocombo;
	GtkWidget *label11;
	GtkWidget *label17;
	GtkWidget *label10;
	GtkWidget *ntsccombo;
	GtkWidget *rendercombo;
	GtkWidget *label9;
	GtkWidget *label2;
	
	videowindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(videowindow), "Video Configuration");
	
	videofixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(videowindow), videofixed);

	scaleamtcombo = gtk_combo_box_text_new ();
	gtk_fixed_put(GTK_FIXED (videofixed), scaleamtcombo, 64, 56);
	gtk_widget_set_size_request(scaleamtcombo, 80, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (scaleamtcombo), "1x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (scaleamtcombo), "2x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (scaleamtcombo), "3x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (scaleamtcombo), "4x");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scaleamtcombo), sSettings->GetScaleAmt());

	scalelabel = gtk_label_new("Scale");
	gtk_fixed_put(GTK_FIXED (videofixed), scalelabel, 8, 64);
	gtk_widget_set_size_request(scalelabel, 47, 17);

	filterlabel = gtk_label_new("Filter");
	gtk_fixed_put(GTK_FIXED (videofixed), filterlabel, 0, 16);
	gtk_widget_set_size_request(filterlabel, 64, 24);

	scalecombo = gtk_combo_box_text_new ();
	gtk_fixed_put (GTK_FIXED (videofixed), scalecombo, 64, 8);
	gtk_widget_set_size_request (scalecombo, 152, 32);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (scalecombo), "None");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (scalecombo), "NTSC");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (scalecombo), "Scale?x");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (scalecombo), "hq?x");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scalecombo), sSettings->GetScale());

	check_fullscreen = gtk_check_button_new_with_mnemonic (_("Fullscreen"));
	gtk_widget_show (check_fullscreen);
	gtk_fixed_put (GTK_FIXED (videofixed), check_fullscreen, 8, 96);
	gtk_widget_set_size_request (check_fullscreen, 96, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fullscreen), sSettings->GetFullscreen());

	unlimitsprcheck = gtk_check_button_new_with_mnemonic ("Unlimited sprites");
	gtk_fixed_put (GTK_FIXED (videofixed), unlimitsprcheck, 8, 120);
	gtk_widget_set_size_request (unlimitsprcheck, 128, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(unlimitsprcheck), sSettings->GetSprlimit());

	videocombo = gtk_combo_box_text_new ();
	gtk_fixed_put (GTK_FIXED (videofixed), videocombo, 344, 104);
	gtk_widget_set_size_request (videocombo, 136, 32);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (videocombo), "Auto");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (videocombo), "NTSC");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (videocombo), "PAL");
	gtk_combo_box_set_active(GTK_COMBO_BOX(videocombo), sSettings->GetVideoMode());

	label11 = gtk_label_new ("Region");
	gtk_fixed_put (GTK_FIXED (videofixed), label11, 280, 104);
	gtk_widget_set_size_request (label11, 56, 32);

	label17 = gtk_label_new ("type");
	gtk_fixed_put (GTK_FIXED (videofixed), label17, 288, 72);
	gtk_widget_set_size_request (label17, 47, 17);

	label10 = gtk_label_new ("NTSC");
	gtk_fixed_put (GTK_FIXED (videofixed), label10, 288, 56);
	gtk_widget_set_size_request (label10, 48, 16);

	ntsccombo = gtk_combo_box_text_new ();
	gtk_fixed_put (GTK_FIXED (videofixed), ntsccombo, 344, 56);
	gtk_widget_set_size_request (ntsccombo, 136, 32);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (ntsccombo), "Composite");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (ntsccombo), "S-Video");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (ntsccombo), "RGB");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ntsccombo), sSettings->GetNtscMode());

	rendercombo = gtk_combo_box_text_new ();
	gtk_fixed_put (GTK_FIXED (videofixed), rendercombo, 344, 8);
	gtk_widget_set_size_request (rendercombo, 136, 32);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (rendercombo), _("Soft"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (rendercombo), _("OpenGL"));
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (rendercombo), _("OpenGL bilinear"));
	gtk_combo_box_set_active(GTK_COMBO_BOX(rendercombo), sSettings->GetRenderType());

	label9 = gtk_label_new (_("Renderer"));
	gtk_fixed_put (GTK_FIXED (videofixed), label9, 252, 16);
	gtk_widget_set_size_request (label9, 96, 24);

	gtk_widget_show_all(videowindow);
	
	g_signal_connect (G_OBJECT(scaleamtcombo), "changed",
		G_CALLBACK (on_scaleamtcombo_changed), NULL);

	g_signal_connect (G_OBJECT(scalecombo), "changed",
		G_CALLBACK (on_scalecombo_changed), NULL);

	g_signal_connect (G_OBJECT(check_fullscreen), "toggled",
		G_CALLBACK (on_check_fullscreen_toggled), NULL);

	g_signal_connect (G_OBJECT(unlimitsprcheck), "toggled",
		G_CALLBACK (on_unlimitsprcheck_toggled), NULL);

	g_signal_connect (G_OBJECT(videocombo), "changed",
		G_CALLBACK (on_videocombo_changed), NULL);

	g_signal_connect (G_OBJECT(ntsccombo), "changed",
		G_CALLBACK (on_ntsccombo_changed), NULL);

	g_signal_connect (G_OBJECT(rendercombo), "changed",
		G_CALLBACK (on_rendercombo_changed), NULL);

	return videowindow;
}

GtkWidget* create_audioconfig (void) {

	GtkWidget *audiowindow;
	GtkWidget *audiofixed;	
	GtkWidget *soundlabel;
	GtkWidget *volumelabel;
	GtkWidget *ratelabel;
	GtkWidget *volumescroll;
	GtkWidget *surrscroll;
	GtkWidget *surrcheck;
	GtkWidget *excitecheck;
	GtkWidget *stereocheck;
	GtkWidget *ratecombo;
	GtkWidget *sndapicombo;
	//GtkWidget *okbutton;

	audiowindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(audiowindow), "Audio Configuration");

	audiofixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(audiowindow), audiofixed);

	soundlabel = gtk_label_new("Sound API:");
	gtk_fixed_put(GTK_FIXED(audiofixed), soundlabel, 16, 16);
	gtk_widget_set_size_request(soundlabel, 112, 16);
	
	sndapicombo = gtk_combo_box_text_new();
	gtk_fixed_put(GTK_FIXED(audiofixed), sndapicombo, 124, 8);
	gtk_widget_set_size_request(sndapicombo, 128, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "SDL");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "ALSA");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "OSS");
	gtk_combo_box_set_active(GTK_COMBO_BOX(sndapicombo), sSettings->GetSndAPI());

	ratelabel = gtk_label_new("Output rate:");
	gtk_fixed_put(GTK_FIXED(audiofixed), ratelabel, 16, 56);
	gtk_widget_set_size_request(ratelabel, 112, 16);
	
	ratecombo = gtk_combo_box_text_new();
	gtk_fixed_put (GTK_FIXED(audiofixed), ratecombo, 124, 48);
	gtk_widget_set_size_request (ratecombo, 128, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "11025");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "22050");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "44100");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "48000");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ratecombo), sSettings->GetRawRate());
	
	volumelabel = gtk_label_new("Volume:");
	gtk_fixed_put(GTK_FIXED(audiofixed), volumelabel, 16, 96);
	gtk_widget_set_size_request(volumelabel, 112, 16);

	volumescroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 5, 0)));
	gtk_widget_show(volumescroll);
	gtk_fixed_put(GTK_FIXED(audiofixed), volumescroll, 124, 92);
	gtk_widget_set_size_request(volumescroll, 128, 24);
	gtk_range_set_value(GTK_RANGE(volumescroll), sSettings->GetVolume());

	surrcheck = gtk_check_button_new_with_mnemonic("Lite Surround:");
	gtk_widget_show(surrcheck);
	gtk_fixed_put(GTK_FIXED(audiofixed), surrcheck, 8, 136);
	gtk_widget_set_size_request(surrcheck, 116, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(surrcheck), sSettings->GetUseSurround());

	surrscroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 5, 0)));
	gtk_widget_show(surrscroll);
	gtk_fixed_put(GTK_FIXED(audiofixed), surrscroll, 124, 136);
	gtk_widget_set_size_request(surrscroll, 128, 24);
	gtk_range_set_value(GTK_RANGE(surrscroll), sSettings->GetSurrMult());

	stereocheck = gtk_check_button_new_with_mnemonic("Stereo");
	gtk_fixed_put(GTK_FIXED(audiofixed), stereocheck, 300, 8);
	gtk_widget_set_size_request(stereocheck, 128, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stereocheck), sSettings->GetStereo());

	excitecheck = gtk_check_button_new_with_mnemonic("Stereo exciter");
	gtk_fixed_put(GTK_FIXED(audiofixed), excitecheck, 300, 48);
	gtk_widget_set_size_request(excitecheck, 128, 21);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(excitecheck), sSettings->GetUseExciter());
	
	/*okbutton = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_fixed_put(GTK_FIXED(audiofixed), okbutton, 300, 96);
	gtk_widget_set_size_request(okbutton, 64, 48);*/
	
	gtk_widget_show_all(audiowindow);
	
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

	g_signal_connect(G_OBJECT(sndapicombo), "changed",
		G_CALLBACK(on_sndapicombo_changed), NULL);

	return audiowindow;
}
