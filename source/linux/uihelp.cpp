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
#include <gdk/gdkx.h>

#include "interface.h"
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
GtkWidget *videowindow;

static GtkWidget *nsftitle, *nsfauthor, *nsfmaker, *text_volume, *scroll_volume, *notebook_main;

static char volumestr[5], surrmulstr[5];

char windowid[24];

extern int xres, yres;

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

	//gtk_widget_set_sensitive(button_nsfplay, FALSE);
	//gtk_widget_set_sensitive(button_nsfstop, TRUE);
}

void on_nsfstop_clicked(GtkButton       *button, gpointer         user_data)
{
	NstStopNsf();

	//gtk_widget_set_sensitive(button_nsfplay, TRUE);
	//gtk_widget_set_sensitive(button_nsfstop, FALSE);
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
		//int i;

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

void on_okbutton_video_clicked(GtkButton *button,  gpointer user_data)
{
	NstPlayGame();
	redraw_request();
	gtk_widget_destroy(videowindow);
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
on_systemcombo_changed                   (GtkComboBox     *combobox,
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

void on_nsfplayer_destroy(GObject *object, gpointer user_data)
{
	NstStopNsf();
	//gtk_widget_destroy(nsfplayer);
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

void UIHelp_Init(int argc, char *argv[], LinuxNst::Settings *settings, LinuxNst::CheatMgr *cheatmgr, int xres, int yres)
{
	//GtkTargetEntry target_entry[1];

	gtk_init(&argc, &argv);

	sSettings = settings;
	sCheatMgr = cheatmgr;

	// crank up our GUI
	mainwindow = create_mainwindow(xres, yres);

	// set up the icon
	app_icon = gdk_pixbuf_new_from_inline(-1, nsticon, FALSE, NULL);
	gtk_window_set_icon(GTK_WINDOW(mainwindow), app_icon);

	// show the window
	gtk_widget_show(mainwindow);
	
}

void UIHelp_Unload(void)
{
	/*// disable the widgets since we unloaded them
	gtk_widget_set_sensitive(button_play, FALSE);
	gtk_widget_set_sensitive(button_nsfplay, FALSE);
	gtk_widget_set_sensitive(button_nsfstop, FALSE);
	gtk_widget_set_sensitive(spin_nsf, FALSE);*/

	// and kill the NSF text since we've unloaded too
	//gtk_label_set_text(GTK_LABEL(nsftitle), " ");
	//gtk_label_set_text(GTK_LABEL(nsfauthor), " ");
	//gtk_label_set_text(GTK_LABEL(nsfmaker), " ");
}

void UIHelp_NSFLoaded(void)
{
	Nsf nsf( emulator );
	
	create_nsfplayer();

	//gtk_widget_set_sensitive(button_nsfplay, TRUE);
	//gtk_widget_set_sensitive(spin_nsf, TRUE);

	// show the NSF info
	gtk_label_set_text(GTK_LABEL(nsftitle), nsf.GetName());
	gtk_label_set_text(GTK_LABEL(nsfauthor), nsf.GetArtist());
	gtk_label_set_text(GTK_LABEL(nsfmaker), nsf.GetCopyright());

	//gtk_spin_button_set_value(GTK_SPIN_BUTTON(spin_nsf), nsf.GetCurrentSong());
	//gtk_spin_button_set_range(GTK_SPIN_BUTTON(spin_nsf), nsf.GetStartingSong(), nsf.GetNumSongs());
}

void UIHelp_GameLoaded(void)
{
	//gtk_widget_set_sensitive(button_play, TRUE);
	//gtk_widget_set_sensitive(button_nsfstop, FALSE);
	//gtk_widget_set_sensitive(button_nsfplay, FALSE);
	//gtk_widget_set_sensitive(spin_nsf, FALSE);
}

// returns NEStopia's icon for child windows to use
GdkPixbuf *UIHelp_GetNSTIcon()
{
	return app_icon;
}

// These functions are dirty hacks to let the main window
// in interace.c call C++ functions. I should probably rethink this later.
void state_load() {
	auxio_do_state_load();
}

void state_save() {
	auxio_do_state_save();
}

void movie_load() {
	auxio_do_movie_load();
}

void movie_record() {
	auxio_do_movie_save();
}

void movie_stop() {
	auxio_do_movie_stop();
}

void redraw_request() {
	redraw_drawingarea(xres, yres);
}

GtkWidget* create_videoconfig (void) {

	GtkWidget *videofixed;

	GtkWidget *scaleamtcombo;
	GtkWidget *scalelabel;
	GtkWidget *filterlabel;
	GtkWidget *scalecombo;
	GtkWidget *check_fullscreen;
	GtkWidget *unlimitsprcheck;
	GtkWidget *videocombo;
	GtkWidget *regionlabel;
	GtkWidget *ntsclabel;
	GtkWidget *ntsccombo;
	GtkWidget *rendercombo;
	GtkWidget *rendererlabel;
	GtkWidget *okbutton;
	
	videowindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_widget_set_size_request(videowindow, 524, 136);
	gtk_window_set_title(GTK_WINDOW(videowindow), "Video Configuration");
	
	videofixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(videowindow), videofixed);

	rendererlabel = gtk_label_new("Renderer:");
	gtk_fixed_put(GTK_FIXED(videofixed), rendererlabel, 8, 16);
	gtk_widget_set_size_request(rendererlabel, 96, 16);

	rendercombo = gtk_combo_box_text_new();
	gtk_fixed_put(GTK_FIXED(videofixed), rendercombo, 128, 8);
	gtk_widget_set_size_request(rendercombo, 144, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rendercombo), "Soft");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rendercombo), "OpenGL");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(rendercombo), "OpenGL bilinear");
	gtk_combo_box_set_active(GTK_COMBO_BOX(rendercombo), sSettings->GetRenderType());

	filterlabel = gtk_label_new("Filter:");
	gtk_fixed_put(GTK_FIXED(videofixed), filterlabel, 8, 56);
	gtk_widget_set_size_request(filterlabel, 96, 16);

	scalecombo = gtk_combo_box_text_new ();
	gtk_fixed_put (GTK_FIXED (videofixed), scalecombo, 128, 48);
	gtk_widget_set_size_request (scalecombo, 144, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "None");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "NTSC");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "Scale?x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "hq?x");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scalecombo), sSettings->GetScale());

	scalelabel = gtk_label_new("Scale:");
	gtk_fixed_put(GTK_FIXED (videofixed), scalelabel, 8, 96);
	gtk_widget_set_size_request(scalelabel, 96, 16);
	
	scaleamtcombo = gtk_combo_box_text_new ();
	gtk_fixed_put(GTK_FIXED(videofixed), scaleamtcombo, 128, 88);
	gtk_widget_set_size_request(scaleamtcombo, 144, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "1x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "2x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "3x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "4x");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scaleamtcombo), sSettings->GetScaleAmt());

	regionlabel = gtk_label_new ("Region:");
	gtk_fixed_put (GTK_FIXED (videofixed), regionlabel, 296, 16);
	gtk_widget_set_size_request (regionlabel, 96, 16);

	videocombo = gtk_combo_box_text_new ();
	gtk_fixed_put (GTK_FIXED (videofixed), videocombo, 392, 8);
	gtk_widget_set_size_request (videocombo, 128, 32);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (videocombo), "Auto");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (videocombo), "NTSC");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (videocombo), "PAL");
	gtk_combo_box_set_active(GTK_COMBO_BOX(videocombo), sSettings->GetVideoMode());

	ntsclabel = gtk_label_new ("NTSC Type:");
	gtk_fixed_put (GTK_FIXED (videofixed), ntsclabel, 296, 56);
	gtk_widget_set_size_request (ntsclabel, 96, 16);

	ntsccombo = gtk_combo_box_text_new();
	gtk_fixed_put (GTK_FIXED (videofixed), ntsccombo, 392, 48);
	gtk_widget_set_size_request (ntsccombo, 128, 32);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (ntsccombo), "Composite");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (ntsccombo), "S-Video");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (ntsccombo), "RGB");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ntsccombo), sSettings->GetNtscMode());

	check_fullscreen = gtk_check_button_new_with_mnemonic ("Fullscreen");
	gtk_widget_show (check_fullscreen);
	gtk_fixed_put (GTK_FIXED (videofixed), check_fullscreen, 8, 128);
	gtk_widget_set_size_request (check_fullscreen, 96, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fullscreen), sSettings->GetFullscreen());

	unlimitsprcheck = gtk_check_button_new_with_mnemonic ("Unlimited sprites");
	gtk_fixed_put (GTK_FIXED (videofixed), unlimitsprcheck, 8, 152);
	gtk_widget_set_size_request (unlimitsprcheck, 128, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(unlimitsprcheck), sSettings->GetSprlimit());
	
	okbutton = gtk_button_new_from_stock(GTK_STOCK_OK);
	gtk_fixed_put (GTK_FIXED(videofixed), okbutton, 448, 128);
	gtk_widget_set_size_request (okbutton, 64, 36);

	gtk_widget_show_all(videowindow);
	
	g_signal_connect(G_OBJECT(scaleamtcombo), "changed",
		G_CALLBACK(on_scaleamtcombo_changed), NULL);
		//G_CALLBACK(redraw_request), NULL);

	g_signal_connect(G_OBJECT(scalecombo), "changed",
		G_CALLBACK(on_scalecombo_changed), NULL);
		//G_CALLBACK(redraw_request), NULL);

	g_signal_connect(G_OBJECT(check_fullscreen), "toggled",
		G_CALLBACK(on_check_fullscreen_toggled), NULL);

	g_signal_connect(G_OBJECT(unlimitsprcheck), "toggled",
		G_CALLBACK(on_unlimitsprcheck_toggled), NULL);

	g_signal_connect(G_OBJECT(videocombo), "changed",
		G_CALLBACK(on_videocombo_changed), NULL);

	g_signal_connect(G_OBJECT(ntsccombo), "changed",
		G_CALLBACK(on_ntsccombo_changed), NULL);

	g_signal_connect(G_OBJECT(rendercombo), "changed",
		G_CALLBACK(on_rendercombo_changed), NULL);
		
	g_signal_connect(G_OBJECT(okbutton), "clicked",
		G_CALLBACK(on_okbutton_video_clicked), NULL);

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
	gtk_fixed_put(GTK_FIXED(audiofixed), sndapicombo, 136, 8);
	gtk_widget_set_size_request(sndapicombo, 128, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "SDL");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "ALSA");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "OSS");
	gtk_combo_box_set_active(GTK_COMBO_BOX(sndapicombo), sSettings->GetSndAPI());

	ratelabel = gtk_label_new("Output rate:");
	gtk_fixed_put(GTK_FIXED(audiofixed), ratelabel, 16, 56);
	gtk_widget_set_size_request(ratelabel, 112, 16);
	
	ratecombo = gtk_combo_box_text_new();
	gtk_fixed_put (GTK_FIXED(audiofixed), ratecombo, 136, 48);
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
	gtk_fixed_put(GTK_FIXED(audiofixed), volumescroll, 136, 92);
	gtk_widget_set_size_request(volumescroll, 128, 24);
	gtk_range_set_value(GTK_RANGE(volumescroll), sSettings->GetVolume());

	surrcheck = gtk_check_button_new_with_mnemonic("Lite Surround:");
	gtk_widget_show(surrcheck);
	gtk_fixed_put(GTK_FIXED(audiofixed), surrcheck, 8, 136);
	gtk_widget_set_size_request(surrcheck, 116, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(surrcheck), sSettings->GetUseSurround());

	surrscroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 5, 0)));
	gtk_widget_show(surrscroll);
	gtk_fixed_put(GTK_FIXED(audiofixed), surrscroll, 136, 136);
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
	
	//okbutton = gtk_button_new_from_stock(GTK_STOCK_OK);
	//gtk_fixed_put(GTK_FIXED(audiofixed), okbutton, 300, 96);
	//gtk_widget_set_size_request(okbutton, 64, 48);
	
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

GtkWidget* create_inputconfig (void) {

	GtkWidget *inputwindow;
	GtkWidget *fixed3;
	GtkWidget *controlcheck;
	GtkWidget *configbutton;
	GtkWidget *configlabel;
	GtkWidget *configcombo;
	
	inputwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(inputwindow), "Input Configuration");

	fixed3 = gtk_fixed_new ();
	gtk_widget_show (fixed3);
	gtk_container_add (GTK_CONTAINER (inputwindow), fixed3);

	controlcheck = gtk_check_button_new_with_mnemonic ("Use controllers");
	gtk_widget_show (controlcheck);
	gtk_fixed_put (GTK_FIXED (fixed3), controlcheck, 8, 16);
	gtk_widget_set_size_request (controlcheck, 136, 24);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(controlcheck), sSettings->GetUseJoypads());

	configbutton = gtk_button_new_with_mnemonic ("Change...");
	gtk_widget_show (configbutton);
	gtk_fixed_put (GTK_FIXED (fixed3), configbutton, 296, 88);
	gtk_widget_set_size_request (configbutton, 112, 40);

	configlabel = gtk_label_new ("");
	gtk_widget_show (configlabel);
	gtk_fixed_put (GTK_FIXED (fixed3), configlabel, 8, 96);
	gtk_widget_set_size_request (configlabel, 264, 24);

	configcombo = gtk_combo_box_text_new ();
	gtk_widget_show (configcombo);
	gtk_fixed_put (GTK_FIXED (fixed3), configcombo, 8, 48);
	gtk_widget_set_size_request (configcombo, 224, 32);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 Up");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 Down");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 Left");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 Right");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 A");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 B");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 START");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 1 SELECT");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 Up");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 Down");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 Left");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 Right");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 A");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 B");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 START");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Player 2 SELECT");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Movie Save");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Movie Load");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Movie Stop");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Reset");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Flip FDS Sides");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Save state");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Load state");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Toggle fullscreen");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Stop game");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Stop game and exit NEStopia");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Start rewinder");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Stop rewinder");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Quicksave slot 1");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Quicksave slot 2");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Quickload slot 1");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Quickload slot 2");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Vs. System coin 1");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT (configcombo), "Vs. System coin 2");
	gtk_combo_box_set_active(GTK_COMBO_BOX(configcombo), sSettings->GetConfigItem());
	
	gtk_widget_show_all(inputwindow);
	
	
	g_signal_connect(G_OBJECT(controlcheck), "toggled",
		G_CALLBACK (on_controlcheck_toggled), NULL);

	g_signal_connect(G_OBJECT(configbutton), "clicked",
		G_CALLBACK (on_configbutton_clicked), NULL);

	g_signal_connect(G_OBJECT(configcombo), "changed",
		G_CALLBACK (on_configcombo_changed), NULL);
	
	return inputwindow;
}

GtkWidget* create_miscconfig (void) {
	
	GtkWidget *miscwindow;
	GtkWidget *miscfixed;
	GtkWidget *systemlabel;
	GtkWidget *spatchlabel;
	GtkWidget *systemcombo;
	GtkWidget *spatchcombo;
	
	miscwindow = gtk_window_new(GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(miscwindow), "Misc Configuration");

	miscfixed = gtk_fixed_new();
	gtk_container_add(GTK_CONTAINER(miscwindow), miscfixed);
	gtk_widget_set_size_request (miscfixed, 288, 112);
	
	systemlabel = gtk_label_new ("Default System:");
	gtk_fixed_put(GTK_FIXED(miscfixed), systemlabel, 8, 24);
	gtk_widget_set_size_request (systemlabel, 128, 16);

	systemcombo = gtk_combo_box_text_new ();
	gtk_fixed_put(GTK_FIXED(miscfixed), systemcombo, 144, 16);
	gtk_widget_set_size_request(systemcombo, 128, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (systemcombo), "NES (NTSC)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (systemcombo), "NES (PAL)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (systemcombo), "Famicom");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (systemcombo), "Dendy");
	gtk_combo_box_set_active(GTK_COMBO_BOX(systemcombo), sSettings->GetPrefSystem());

	spatchlabel = gtk_label_new("Soft patching:");
	gtk_fixed_put(GTK_FIXED(miscfixed), spatchlabel, 8, 64);
	gtk_widget_set_size_request(spatchlabel, 128, 16);

	spatchcombo = gtk_combo_box_text_new();
	gtk_fixed_put(GTK_FIXED(miscfixed), spatchcombo, 144, 56);
	gtk_widget_set_size_request(spatchcombo, 128, 32);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(spatchcombo), "Off");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(spatchcombo), "On");
	gtk_combo_box_set_active(GTK_COMBO_BOX(spatchcombo), sSettings->GetSoftPatch());
	
	gtk_widget_show_all(miscwindow);
	
	g_signal_connect(G_OBJECT(systemcombo), "changed",
		G_CALLBACK(on_systemcombo_changed), NULL);

	g_signal_connect(G_OBJECT(spatchcombo), "changed",
		G_CALLBACK(on_spatchcombo_changed), NULL);
	
	return miscwindow;
}

GtkWidget* create_nsfplayer (void) {
	
	GtkWidget *nsfplayer;
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
}

GtkWidget* create_about (void) {

	char svgpath[1024];
	sprintf(svgpath, "%s/icons/nestopia.svg", DATADIR);
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(svgpath, 256, 256, NULL);
	
	GtkWidget *aboutdialog = gtk_about_dialog_new();
	
	gtk_about_dialog_set_logo(GTK_ABOUT_DIALOG(aboutdialog), pixbuf);
	gtk_about_dialog_set_program_name(GTK_ABOUT_DIALOG(aboutdialog), "Nestopia Undead");
	gtk_about_dialog_set_version(GTK_ABOUT_DIALOG(aboutdialog), "1.43");
	gtk_about_dialog_set_comments(GTK_ABOUT_DIALOG(aboutdialog), "An accurate Nintendo Entertainment System Emulator");
	gtk_about_dialog_set_website(GTK_ABOUT_DIALOG(aboutdialog), "http://0ldsk00l.ca/");
	gtk_about_dialog_set_copyright(GTK_ABOUT_DIALOG(aboutdialog), "(c) 2012, R. Danbrook\n(c) 2007-2008, R. Belmont\n(c) 2003-2008, Martin Freij\n\nIcon based on art from Trollekop");
	g_object_unref(pixbuf), pixbuf = NULL;
	gtk_dialog_run(GTK_DIALOG(aboutdialog));
	gtk_widget_destroy(aboutdialog);
	
	return aboutdialog;
}

// Ripped this straight out of FCEUX and Gens/GS

unsigned short GDKToSDLKeyval(int gdk_key)
{
	if (!(gdk_key & 0xFF00))
	{
		// ASCII symbol.
		// SDL and GDK use the same values for these keys.
		
		// Make sure the key value is lowercase.
		gdk_key = tolower(gdk_key);
		
		// Return the key value.
		return gdk_key;
	}
	
	if (gdk_key & 0xFFFF0000)
	{
		// Extended X11 key. Not supported by SDL.
#ifdef GDK_WINDOWING_X11
		fprintf(stderr, "Unhandled extended X11 key: 0x%08X (%s)", gdk_key, XKeysymToString(gdk_key));
#else
		fprintf(stderr, "Unhandled extended key: 0x%08X\n", gdk_key);
#endif
		return 0;
	}
	
	// Non-ASCII symbol.
	static const uint16_t gdk_to_sdl_table[0x100] =
	{
		// 0x00 - 0x0F
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		SDLK_BACKSPACE, SDLK_TAB, SDLK_RETURN, SDLK_CLEAR,
		0x0000, SDLK_RETURN, 0x0000, 0x0000,
		
		// 0x10 - 0x1F
		0x0000, 0x0000, 0x0000, SDLK_PAUSE,
		SDLK_SCROLLOCK, SDLK_SYSREQ, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, SDLK_ESCAPE,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x20 - 0x2F
		SDLK_COMPOSE, 0x0000, 0x0000, 0x0000,
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
		0x0000, SDLK_PRINT, 0x0000, SDLK_INSERT,
		SDLK_UNDO, 0x0000, 0x0000, SDLK_MENU,
		0x0000, SDLK_HELP, SDLK_BREAK, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		
		// 0x70 - 0x7F [mostly unused, except for Alt Gr and Num Lock]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, SDLK_MODE, SDLK_NUMLOCK,
		
		// 0x80 - 0x8F [mostly unused, except for some numeric keypad keys]
		SDLK_KP5, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, SDLK_KP_ENTER, 0x0000, 0x0000,
		
		// 0x90 - 0x9F
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, SDLK_KP7, SDLK_KP4, SDLK_KP8,
		SDLK_KP6, SDLK_KP2, SDLK_KP9, SDLK_KP3,
		SDLK_KP1, SDLK_KP5, SDLK_KP0, SDLK_KP_PERIOD,
		
		// 0xA0 - 0xAF
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, SDLK_KP_MULTIPLY, SDLK_KP_PLUS,
		0x0000, SDLK_KP_MINUS, SDLK_KP_PERIOD, SDLK_KP_DIVIDE,
		
		// 0xB0 - 0xBF
		SDLK_KP0, SDLK_KP1, SDLK_KP2, SDLK_KP3,
		SDLK_KP4, SDLK_KP5, SDLK_KP6, SDLK_KP7,
		SDLK_KP8, SDLK_KP9, 0x0000, 0x0000,
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
		SDLK_RCTRL, SDLK_CAPSLOCK, 0x0000, SDLK_LMETA,
		SDLK_RMETA, SDLK_LALT, SDLK_RALT, SDLK_LSUPER,
		SDLK_RSUPER, 0x0000, 0x0000, 0x0000,
		
		// 0xF0 - 0xFF [mostly unused, except for Delete]
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, 0x0000,
		0x0000, 0x0000, 0x0000, SDLK_DELETE,		
	};
	
	unsigned short sdl_key = gdk_to_sdl_table[gdk_key & 0xFF];
	if (sdl_key == 0)
	{
		// Unhandled GDK key.
		fprintf(stderr, "Unhandled GDK key: 0x%04X (%s)", gdk_key, gdk_keyval_name(gdk_key));
		return 0;
	}
	
	// ignore pause and screenshot hotkeys since they is handled by GTK+ as accelerators
	/*if (sdl_key == Hotkeys[HK_PAUSE] || sdl_key == Hotkeys[HK_SCREENSHOT] || 
		sdl_key == Hotkeys[HK_SAVE_STATE] || sdl_key == Hotkeys[HK_LOAD_STATE])
		return 0;*/
	
	return sdl_key;
}

// Function adapted from Gens/GS (source/gens/input/input_sdl.c)
gint convertKeypress(GtkWidget *grab, GdkEventKey *event, gpointer user_data)
{
	SDL_Event sdlev;
	SDLKey sdlkey;
	int keystate;
	
	switch (event->type)
	{
		case GDK_KEY_PRESS:
			sdlev.type = SDL_KEYDOWN;
			sdlev.key.state = SDL_PRESSED;
			keystate = 1;
			break;
		
		case GDK_KEY_RELEASE:
			sdlev.type = SDL_KEYUP;
			sdlev.key.state = SDL_RELEASED;
			keystate = 0;
			break;
		
		default:
			fprintf(stderr, "Unhandled GDK event type: %d", event->type);
			return FALSE;
	}
	
	// Convert this keypress from GDK to SDL.
	sdlkey = (SDLKey)GDKToSDLKeyval(event->keyval);
	
	// Create an SDL event from the keypress.
	sdlev.key.keysym.sym = sdlkey;
	if (sdlkey != 0)
	{
		SDL_PushEvent(&sdlev);
		
		// Only let the emulator handle the key event if this window has the input focus.
		//if(keystate == 0 || gtk_window_is_active(GTK_WINDOW(mainwindow)))
		//{
		//	#if SDL_VERSION_ATLEAST(1, 3, 0)
		//	SDL_GetKeyboardState(NULL)[SDL_GetScancodeFromKey(sdlkey)] = keystate;
		//	#else
			SDL_GetKeyState(NULL)[sdlkey] = keystate;
		//	#endif
		//}
	}
	
	// Allow GTK+ to process this key.
	return FALSE;
}

void set_window_id(char* sdlwindowid) {
	//printf("%s\n", sdlwindowid);
	sprintf(windowid, "%s", sdlwindowid);
}
