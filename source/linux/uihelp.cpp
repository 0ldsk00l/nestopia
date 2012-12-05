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
#include <gdk/gdkkeysyms.h>
#include <gdk/gdkx.h>

#include "interface.h"
#include "callbacks.h"
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
GtkWidget *configwindow;

static GtkWidget *nsfplayer, *nsftitle, *nsfauthor, *nsfmaker, *text_volume, *scroll_volume;

static char volumestr[5], surrmulstr[5];

char windowid[24];

int playernumber = 0;

extern int xres, yres;

bool wasplaying = 0;

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
		get_screen_res();
	}

	redraw_request();
}

void
on_check_fullscreen_toggled             (GtkToggleButton *togglebutton,
                                        gpointer         user_data)
{
	sSettings->SetFullscreen(gtk_toggle_button_get_active(togglebutton));
}

void on_check_fsnativeres_toggled(GtkToggleButton *togglebutton, gpointer user_data) {
	sSettings->SetFsNativeRes(gtk_toggle_button_get_active(togglebutton));
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

void playercombo_changed(GtkComboBox *combobox, gpointer user_data) {
	playernumber = gtk_combo_box_get_active(combobox);
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
	 int curItem = sSettings->GetConfigItem();
	 //printf("%d\n", curItem);
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

void inputcfg_clicked(GtkButton *button, int data) {
printf("Inputcfg\n");
	if (playernumber == 0 && data < 16) {	// Player 1
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

	//gtk_init(&argc, &argv);

	sSettings = settings;
	sCheatMgr = cheatmgr;

	// crank up our GUI
	mainwindow = create_mainwindow(xres, yres);

	// set up the icon
	char iconpath[1024];
	sprintf(iconpath, "%s/icons/nestopia.svg", DATADIR);
	
	// Load the icon from local source dir if make install hasn't been done
	struct stat iconstat;
	if (stat(iconpath, &iconstat) == -1) {
		sprintf(iconpath, "source/linux/icons/nestopia.svg");
	}

	app_icon = gdk_pixbuf_new_from_file(iconpath, NULL);
	gtk_window_set_icon(GTK_WINDOW(mainwindow), app_icon);

	// show the window
	gtk_widget_show(mainwindow);
	
}

void UIHelp_NSFLoaded(void)
{
	Nsf nsf( emulator );
	
	create_nsfplayer();

	// show the NSF info
	gtk_label_set_text(GTK_LABEL(nsftitle), nsf.GetName());
	gtk_label_set_text(GTK_LABEL(nsfauthor), nsf.GetArtist());
	gtk_label_set_text(GTK_LABEL(nsfmaker), nsf.GetCopyright());
}

// return the icon for alternate windows to use
GdkPixbuf *get_icon() {
	return app_icon;
}

// These functions are dirty hacks to let the main window
// in interace.c call C++ functions. I should probably rethink this later.
void pause_clicked() {
	bool playing = NstIsPlaying();
	if (playing) {
		wasplaying = 1;
		NstStopPlaying();
	}
}

void reset_clicked() {
	NstReset();
}

void fullscreen_clicked() {
	ToggleFullscreen();
}

void state_load() {
	auxio_do_state_load();
}

void state_save() {
	auxio_do_state_save();
}

void flipdisk_clicked() {
	FlipFDSDisk();
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

GtkWidget* create_config(void) {

	// Pause if playing
	bool playing = NstIsPlaying();
	if (playing) {
		NstStopPlaying();
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
    gtk_combo_box_set_active(GTK_COMBO_BOX(rendercombo), sSettings->GetRenderType());
    
    GtkWidget *vidsettingslabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(vidsettingslabel), "<b>Settings</b>");
    
	GtkWidget *scalebox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *scalelabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "Scaler:", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	GtkWidget *scalecombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "None");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "NTSC");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "Scale?x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scalecombo), "hq?x");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scalecombo), sSettings->GetScale());
	gtk_box_pack_start(GTK_BOX(scalebox), scalelabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(scalebox), scalecombo, FALSE, FALSE, 0);
	
	GtkWidget *scaleamtbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *scaleamtlabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "Multiplier:", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	GtkWidget *scaleamtcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "1x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "2x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "3x");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(scaleamtcombo), "4x");
	gtk_combo_box_set_active(GTK_COMBO_BOX(scaleamtcombo), sSettings->GetScaleAmt());
	gtk_box_pack_start(GTK_BOX(scaleamtbox), scaleamtlabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(scaleamtbox), scaleamtcombo, FALSE, FALSE, 0);
	
	GtkWidget *ntscbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *ntsclabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "NTSC Type:", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	GtkWidget *ntsccombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(ntsccombo), "Composite");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(ntsccombo), "S-Video");
	gtk_combo_box_text_append_text (GTK_COMBO_BOX_TEXT(ntsccombo), "RGB");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ntsccombo), sSettings->GetNtscMode());
	gtk_box_pack_start(GTK_BOX(ntscbox), ntsclabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(ntscbox), ntsccombo, FALSE, FALSE, 0);
	
	GtkWidget *check_fullscreen = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Fullscreen", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fullscreen), sSettings->GetFullscreen());
	
	GtkWidget *check_fsnativeres = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Native Resolution (Fullscreen)", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(check_fsnativeres), sSettings->GetFsNativeRes());

	GtkWidget *unlimitsprcheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Unlimited Sprites", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(unlimitsprcheck), sSettings->GetSprlimit());

	gtk_box_pack_start(GTK_BOX(videobox), renderlabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), rendercombo, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), vidsettingslabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), scalebox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), scaleamtbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), ntscbox, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), check_fullscreen, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), check_fsnativeres, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(videobox), unlimitsprcheck, FALSE, FALSE, 0);
	//End of the Video stuff

    // The Audio stuff
    GtkWidget *audiobox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 0);
    
    GtkWidget *sndapilabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(sndapilabel), "<b>Sound API</b>");

    GtkWidget *sndapicombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
    gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "SDL");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "ALSA");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (sndapicombo), "OSS");
	gtk_combo_box_set_active(GTK_COMBO_BOX(sndapicombo), sSettings->GetSndAPI());
	
	GtkWidget *ratecombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "11025 Hz");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "22050 Hz");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "44100 Hz");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT (ratecombo), "48000 Hz");
	gtk_combo_box_set_active(GTK_COMBO_BOX(ratecombo), sSettings->GetRawRate());
	
	GtkWidget *audsettingslabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(audsettingslabel), "<b>Settings</b>");

	GtkWidget *volumebox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *volumelabel = gtk_widget_new(GTK_TYPE_LABEL, "label", "Volume", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	GtkWidget *volumescroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT (gtk_adjustment_new (0, 0, 100, 1, 5, 0)));
	gtk_widget_set_size_request(volumescroll, 128, 24);
	gtk_range_set_value(GTK_RANGE(volumescroll), sSettings->GetVolume());
	gtk_box_pack_start(GTK_BOX(volumebox), volumelabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(volumebox), volumescroll, FALSE, FALSE, 0);

	GtkWidget *surrbox = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 0);
	GtkWidget *surrcheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Lite Surround", "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(surrcheck), sSettings->GetUseSurround());
	GtkWidget *surrscroll = gtk_scrollbar_new(GTK_ORIENTATION_HORIZONTAL, GTK_ADJUSTMENT(gtk_adjustment_new (0, 0, 100, 1, 5, 0)));
	gtk_widget_set_size_request(surrscroll, 128, 24);
	gtk_range_set_value(GTK_RANGE(surrscroll), sSettings->GetSurrMult());
	gtk_box_pack_start(GTK_BOX(surrbox), surrcheck, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(surrbox), surrscroll, FALSE, FALSE, 0);

	GtkWidget *stereocheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Stereo", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(stereocheck), sSettings->GetStereo());

	GtkWidget *excitecheck = gtk_widget_new(GTK_TYPE_CHECK_BUTTON, "label", "Stereo Exciter", "halign", GTK_ALIGN_START, "margin-left", 10, NULL);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(excitecheck), sSettings->GetUseExciter());

	gtk_box_pack_start(GTK_BOX(audiobox), sndapilabel, FALSE, FALSE, 0);
	gtk_box_pack_start(GTK_BOX(audiobox), sndapicombo, FALSE, FALSE, 0);
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
	sprintf(svgpath, "%s/icons/nespad.svg", DATADIR);
	
	// Load the NES pad svg from local source dir if make install hasn't been done
	struct stat svgstat;
	if (stat(svgpath, &svgstat) == -1) {
		sprintf(svgpath, "source/linux/icons/nespad.svg");
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
	gtk_combo_box_set_active(GTK_COMBO_BOX(videocombo), sSettings->GetVideoMode());
    
    GtkWidget *systemlabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(systemlabel), "<b>Default System</b>");
	GtkWidget *systemcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "NES (NTSC)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "NES (PAL)");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "Famicom");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(systemcombo), "Dendy");
	gtk_combo_box_set_active(GTK_COMBO_BOX(systemcombo), sSettings->GetPrefSystem());

	GtkWidget *spatchlabel = gtk_widget_new(GTK_TYPE_LABEL, "xalign", 0.0, "margin-top", 10, "margin-bottom", 5, "margin-left", 10, NULL);
	gtk_label_set_markup(GTK_LABEL(spatchlabel), "<b>Soft Patching</b>");
	GtkWidget *spatchcombo = gtk_widget_new(GTK_TYPE_COMBO_BOX_TEXT, "halign", GTK_ALIGN_START, "margin-bottom", 5, "margin-left", 10, "margin-right", 10, NULL);
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(spatchcombo), "Off");
	gtk_combo_box_text_append_text(GTK_COMBO_BOX_TEXT(spatchcombo), "On");
	gtk_combo_box_set_active(GTK_COMBO_BOX(spatchcombo), sSettings->GetSoftPatch());

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

	g_signal_connect(G_OBJECT(check_fullscreen), "toggled",
		G_CALLBACK(on_check_fullscreen_toggled), NULL);
		
	g_signal_connect(G_OBJECT(check_fsnativeres), "toggled",
		G_CALLBACK(on_check_fsnativeres_toggled), NULL);

	g_signal_connect(G_OBJECT(unlimitsprcheck), "toggled",
		G_CALLBACK(on_unlimitsprcheck_toggled), NULL);

	g_signal_connect(G_OBJECT(videocombo), "changed",
		G_CALLBACK(on_videocombo_changed), NULL);

	g_signal_connect(G_OBJECT(ntsccombo), "changed",
		G_CALLBACK(on_ntsccombo_changed), NULL);

	g_signal_connect(G_OBJECT(rendercombo), "changed",
		G_CALLBACK(on_rendercombo_changed), NULL);

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

	g_signal_connect(G_OBJECT(sndapicombo), "changed",
		G_CALLBACK(on_sndapicombo_changed), NULL);

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

GtkWidget* create_nsfplayer (void) {

	// Pause if playing
	bool playing = NstIsPlaying();
	if (playing) {
		NstStopPlaying();
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
}

GtkWidget* create_about (void) {

	char svgpath[1024];
	sprintf(svgpath, "%s/icons/nestopia.svg", DATADIR);
	
	// Load the SVG from local source dir if make install hasn't been done
	struct stat svgstat;
	if (stat(svgpath, &svgstat) == -1) {
		sprintf(svgpath, "source/linux/icons/nestopia.svg");
	}
	
	GdkPixbuf *pixbuf = gdk_pixbuf_new_from_file_at_size(svgpath, 192, 192, NULL);
	
	GtkWidget *aboutdialog = gtk_about_dialog_new();
	
	GdkPixbuf *app_icon = get_icon();
	gtk_window_set_icon(GTK_WINDOW(aboutdialog), app_icon);
	
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
