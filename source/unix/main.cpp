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
#include <libgen.h>
#ifdef MINGW
#include <io.h>
#endif

#include <SDL.h>
#include <SDL_endian.h>

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
#include "cli.h"
//#include "gtkui.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "fileio.h"
#include "cheats.h"
#include "config.h"
#include "cursor.h"

using namespace Nes::Api;
using namespace LinuxNst;

// base class, all interfaces derives from this
Emulator emulator;

int playing = 0, loaded = 0;
static int nst_quit = 0, nsf_mode = 0, state_save = 0, state_load = 0, movie_save = 0, movie_load = 0, movie_stop = 0;
int schedule_stop = 0;

char nstdir[256], savedir[512];
static char savename[512], gamebasename[512];
char rootname[512], lastarchname[512];
char msgbuf[512];

static CheatMgr *sCheatMgr;

static Video::Output *cNstVideo;
static Sound::Output *cNstSound;
static Input::Controllers *cNstPads;
static Cartridge::Database::Entry dbentry;

//extern GtkWidget *mainwindow, *statusbar;
//extern char windowid[24];
extern dimensions basesize, rendersize;
extern void	*videobuf;

extern settings conf;

// get the favored system selected by the user
static Machine::FavoredSystem get_favored_system(void)
{
	switch (conf.misc_default_system) {
		case 0:
			return Machine::FAVORED_NES_NTSC;
			break;

		case 1:
			return Machine::FAVORED_NES_PAL;
			break;

		case 2:
			return Machine::FAVORED_FAMICOM;
			break;

		case 3:
			return Machine::FAVORED_DENDY;
			break;
	}

	return Machine::FAVORED_NES_NTSC;
}

// *******************
// emulation callbacks
// *******************

// called right before Nestopia is about to write pixels
static bool NST_CALLBACK VideoLock(void* userData, Video::Output& video)
{
	if (nsf_mode) return false;

	video.pitch = video_lock_screen(video.pixels);
	return true; // true=lock success, false=lock failed (Nestopia will carry on but skip video)
}

// called right after Nestopia has finished writing pixels (not called if previous lock failed)
static void NST_CALLBACK VideoUnlock(void* userData, Video::Output& video)
{
	if (nsf_mode) return;

	video_unlock_screen(video.pixels);
}

static bool NST_CALLBACK SoundLock(void* userData, Sound::Output& sound) {
	return true;
}

static void NST_CALLBACK SoundUnlock(void* userData, Sound::Output& sound) {
	audio_play();
}

// do a "partial" shutdown
static void nst_unload(void)
{
	Machine machine(emulator);

	// if nothing's loaded, do nothing
	if (!loaded)
	{
		return;
	}

	// power down the emulated NES
	std::cout << "Powering down the emulated machine\n";
	machine.Power(false);

	// unload the cart
	machine.Unload();

	// erase any cheats
	//sCheatMgr->Unload();
}

// returns if we're currently playing a game or NSF
bool NstIsPlaying() {
	return playing;
}

bool NstIsLoaded() {
	return loaded;
}

// shuts everything down
void NstStopPlaying()
{
	if (playing)
	{
		int i;

		// kill any movie
		fileio_do_movie_stop();

		// get machine interface...
		Machine machine(emulator);
		
		audio_deinit();
	}

	playing = 0;

	cursor_set_default();
}

// generate the filename for quicksave files
std::string StrQuickSaveFile(int isvst) {
	
	std::ostringstream ossFile;
	ossFile << nstdir;
	ossFile << "state";
	
	ossFile << "/" << std::setbase(16) << std::setfill('0') << std::setw(8)
		<< basename(gamebasename) << std::string("_") << isvst << ".nst";
	
	return ossFile.str();
}

void FlipFDSDisk() {
	Fds fds(emulator);

	if (fds.CanChangeDiskSide()) {
		fds.ChangeSide();
		print_fds_info();
	}
}

void SwitchFDSDisk() {
	Fds fds(emulator);
	
	int currentdisk = fds.GetCurrentDisk();
	
	// If it's a multi-disk game, eject and insert the other disk
	if (fds.GetNumDisks() > 1) {
		fds.EjectDisk();
		fds.InsertDisk(!currentdisk, 0);
		print_fds_info();
	}
}

void print_fds_info() {
	Fds fds(emulator);

	char* disk;
	char* side;

	fds.GetCurrentDisk() == 0 ? disk = "1" : disk = "2";
	fds.GetCurrentDiskSide() == 0 ? side = "A" : side = "B";

	snprintf(msgbuf, sizeof(msgbuf), "Fds: Disk %s Side %s", disk, side);
	print_message(msgbuf);
}

void print_message(char* message) {
	printf("%s\n", message);
	//gtk_statusbar_push(GTK_STATUSBAR(statusbar), 0, message);
}

// save state to memory slot
void QuickSave(int isvst)
{
	std::string strFile = StrQuickSaveFile(isvst);

	Machine machine( emulator );
	std::ofstream os(strFile.c_str());
	// use "NO_COMPRESSION" to make it easier to hack save states
	Nes::Result res = machine.SaveState(os, Nes::Api::Machine::USE_COMPRESSION);
	snprintf(msgbuf, sizeof(msgbuf), "State Saved: %s", strFile.c_str());
	print_message(msgbuf);
}


// restore state from memory slot
void QuickLoad(int isvst)
{
	std::string strFile = StrQuickSaveFile(isvst);
	
	struct stat qloadstat;
	if (stat(strFile.c_str(), &qloadstat) == -1) {
		snprintf(msgbuf, sizeof(msgbuf), "No State to Load");
		print_message(msgbuf);
		return;
	}

	Machine machine( emulator );
	std::ifstream is(strFile.c_str());
	Nes::Result res = machine.LoadState(is);
	snprintf(msgbuf, sizeof(msgbuf), "State Loaded: %s", strFile.c_str());
	print_message(msgbuf);
}


// start playing
void NstPlayGame(void)
{
	// initialization
	video_init();
	audio_init();
	SetupInput();

	// apply any cheats into the engine
	//sCheatMgr->Enable();

	cNstVideo = new Video::Output;
	cNstSound = new Sound::Output;
	cNstPads  = new Input::Controllers;
	
	audio_set_params(cNstSound);
	audio_unpause();

	schedule_stop = 0;
	playing = 1;
}

void NstSoftReset() {
	Machine machine(emulator);
	Fds fds(emulator);
	machine.Reset(false);

	// put the disk system back to disk 0 side 0
	fds.EjectDisk();
	fds.InsertDisk(0, 0);
}

void NstHardReset() {
	Machine machine(emulator);
	Fds fds(emulator);
	machine.Reset(true);

	// put the disk system back to disk 0 side 0
	fds.EjectDisk();
	fds.InsertDisk(0, 0);
}

// schedule a NEStopia quit
void NstScheduleQuit() {
	nst_quit = 1;
}

// logging callback called by the core
static void NST_CALLBACK DoLog(void *userData, const char *string, unsigned long int length)
{
	fprintf(stderr, "%s", string);
}

// for various file operations, usually called during image file load, power on/off and reset
static void NST_CALLBACK DoFileIO(void *userData, User::File& file)
{
	unsigned char *compbuffer;
	int compsize, compoffset;
	char mbstr[512];

	switch (file.GetAction())
	{
		case User::File::LOAD_ROM:
			wcstombs(mbstr, file.GetName(), 511);

			if (fileio_load_archive(lastarchname, &compbuffer, &compsize, &compoffset, (const char *)mbstr))
			{
				file.SetContent((const void*)&compbuffer[compoffset], (unsigned long int)compsize);

				free(compbuffer);
			}				
			break;

		case User::File::LOAD_SAMPLE:
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU:
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88:
		case User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS:
		case User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU:
		case User::File::LOAD_SAMPLE_AEROBICS_STUDIO:
			wcstombs(mbstr, file.GetName(), 511);

			if (fileio_load_archive(lastarchname, &compbuffer, &compsize, &compoffset, (const char *)mbstr))
			{
				int chan, bits, rate;

				if (!strncmp((const char *)compbuffer, "RIFF", 4))
				{
					chan = compbuffer[20] | compbuffer[21]<<8;
					rate = compbuffer[24] | compbuffer[25]<<8 | compbuffer[26]<<16 | compbuffer[27]<<24;
					bits = compbuffer[34] | compbuffer[35]<<8; 

//					std::cout << "WAV has " << chan << " chan, " << bits << " bits per sample, rate = " << rate << "\n";

					file.SetSampleContent((const void*)&compbuffer[compoffset], (unsigned long int)compsize, (chan == 2) ? true : false, bits, rate);
				}

				free(compbuffer);
			}				
			break;

		case User::File::LOAD_BATTERY: // load in battery data from a file
		case User::File::LOAD_EEPROM: // used by some Bandai games, can be treated the same as battery files
		case User::File::LOAD_TAPE: // for loading Famicom cassette tapes
		case User::File::LOAD_TURBOFILE: // for loading turbofile data
		{
			//int size;
			FILE *f;

			f = fopen(savename, "rb");
			if (!f)
			{
				return;
			}
			fseek(f, 0, SEEK_END);
			//size = ftell(f);
			fclose(f);

			std::ifstream batteryFile( savename, std::ifstream::in|std::ifstream::binary );

			if (batteryFile.is_open())
			{
				file.SetContent( batteryFile );
			}
			break;
		}

		case User::File::SAVE_BATTERY: // save battery data to a file
		case User::File::SAVE_EEPROM: // can be treated the same as battery files
		case User::File::SAVE_TAPE: // for saving Famicom cassette tapes
		case User::File::SAVE_TURBOFILE: // for saving turbofile data
		{
			std::ofstream batteryFile( savename, std::ifstream::out|std::ifstream::binary );
			const void* savedata;
			unsigned long savedatasize;

			file.GetContent( savedata, savedatasize );

			if (batteryFile.is_open())
				batteryFile.write( (const char*) savedata, savedatasize );

			break;
		}

		case User::File::LOAD_FDS: // for loading modified Famicom Disk System files
		{
			char fdsname[512];

			snprintf(fdsname, sizeof(fdsname), "%s.ups", rootname);
			
			std::ifstream batteryFile( fdsname, std::ifstream::in|std::ifstream::binary );

			// no ups, look for ips
			if (!batteryFile.is_open())
			{
				snprintf(fdsname, sizeof(fdsname), "%s.ips", rootname);

				std::ifstream batteryFile( fdsname, std::ifstream::in|std::ifstream::binary );

				if (!batteryFile.is_open())
				{
					return;
				}

				file.SetPatchContent(batteryFile);
				return;
			}

			file.SetPatchContent(batteryFile);
			break;
		}

		case User::File::SAVE_FDS: // for saving modified Famicom Disk System files
		{
			char fdsname[512];

			snprintf(fdsname, sizeof(fdsname), "%s.ups", rootname);

			std::ofstream fdsFile( fdsname, std::ifstream::out|std::ifstream::binary );

			if (fdsFile.is_open())
				file.GetPatchContent( User::File::PATCH_UPS, fdsFile );

			break;
		}
	}
}

/*static void cleanup_after_io(void) {
	gtk_main_iteration_do(FALSE);
	gtk_main_iteration_do(FALSE);
	gtk_main_iteration_do(FALSE);
}*/

void nst_set_dirs() {
	// Set up system directories
#ifdef MINGW
	snprintf(nstdir, sizeof(nstdir), "");
#else
	// create system directory if it doesn't exist
	snprintf(nstdir, sizeof(nstdir), "%s/.nestopia/", getenv("HOME"));
	if (mkdir(nstdir, 0755) && errno != EEXIST) {	
		fprintf(stderr, "Failed to create %s: %d\n", nstdir, errno);
	}
#endif
	// create save and state directories if they don't exist
	char dirstr[256];
	snprintf(dirstr, sizeof(dirstr), "%ssave", nstdir);
#ifdef MINGW	
	if (mkdir(dirstr) && errno != EEXIST) {
#else
	if (mkdir(dirstr, 0755) && errno != EEXIST) {
#endif
		fprintf(stderr, "Failed to create %s: %d\n", dirstr, errno);
	}

	snprintf(dirstr, sizeof(dirstr), "%sstate", nstdir);
#ifdef MINGW	
	if (mkdir(dirstr) && errno != EEXIST) {
#else
	if (mkdir(dirstr, 0755) && errno != EEXIST) {
#endif
		fprintf(stderr, "Failed to create %s: %d\n", dirstr, errno);
	}
}

int main(int argc, char *argv[]) {
	
	static SDL_Event event;
	int i;
	void* userData = (void*) 0xDEADC0DE;

	// Set up directories
	nst_set_dirs();
	
	// read the config file
	config_file_read();
	
	if (argc == 1 && conf.misc_disable_gui) {
		// Show usage and free config 
		cli_show_usage();
		return 0;
	}
	
	cli_handle_command(argc, argv);
	
	playing = 0;
	videobuf = NULL;
	
	// Initialize File input/output routines
	fileio_init();
	
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	// Initialize input and read input config
	input_init();
	input_config_read();
	
	// Set up the video parameters
	video_set_params();
	
	// Initialize GTK+
	/*gtk_init(&argc, &argv);
	
	if (conf.misc_disable_gui) {
		// do nothing at this point
	}
	// Don't show a GUI if it has been disabled in the config
	else {
		gtkui_init(argc, argv, rendersize.w, rendersize.h);
	}*/

	// Create the game window
	video_create();
	
	// setup video lock/unlock callbacks
	Video::Output::lockCallback.Set( VideoLock, userData );
	Video::Output::unlockCallback.Set( VideoUnlock, userData );
	
	// set up audio lock/unlock callbacks
	Sound::Output::lockCallback.Set(SoundLock, userData);
	Sound::Output::unlockCallback.Set(SoundUnlock, userData);

	// misc callbacks (for others, see NstApuUser.hpp)
	User::fileIoCallback.Set( DoFileIO, userData );
	User::logCallback.Set( DoLog, userData );

	// try to load the FDS BIOS
	fileio_set_fds_bios();

	// and the NST database
	fileio_load_db();

	// attempt to load and autostart a file specified on the commandline
	if (argc > 1) {
		NstLoadGame(argv[argc - 1]);

		if (loaded)
		{
			NstPlayGame();
		}
	}

	nst_quit = 0;
	while (!nst_quit)
	{
		/*while (gtk_events_pending())
		{
			gtk_main_iteration();
		}*/
		
		if (playing)
		{
				//gtk_main_iteration_do(FALSE);

			 	while (SDL_PollEvent(&event))
				{
					switch (event.type)
					{
						case SDL_QUIT:
							schedule_stop = 1;
							return 0; // FIX
							break;
						
						case SDL_KEYDOWN:
						case SDL_KEYUP:
							// ignore num lock, caps lock, and "mode" (whatever that is)
							//event.key.keysym.mod = (SDLMod)((int)event.key.keysym.mod & (~(KMOD_NUM | KMOD_CAPS | KMOD_MODE)));
							
							// (intentional fallthrough)
						case SDL_JOYHATMOTION:
						case SDL_JOYAXISMOTION:
						case SDL_JOYBUTTONDOWN:
						case SDL_JOYBUTTONUP:
						case SDL_MOUSEBUTTONDOWN:
						case SDL_MOUSEBUTTONUP:
							input_process(cNstPads, event);
							break;
					}	
				}
				//}

				if (NES_SUCCEEDED(Rewinder(emulator).Enable(true)))
				{
					Rewinder(emulator).EnableSound(true);
				}

			//if (timing_check()) {
				emulator.Execute(cNstVideo, cNstSound, cNstPads);
				//emulator.Execute(cNstVideo, NULL, cNstPads);
			//}
			

			if (state_save)
			{
				fileio_do_state_save();
				state_save = 0;
				//cleanup_after_io();
			}

			if (state_load)
			{
				fileio_do_state_load();
				state_load = 0;
				//cleanup_after_io();
			}

			if (movie_load)
			{
				fileio_do_movie_load();
				movie_load = 0;
				//cleanup_after_io();
			}

			if (movie_save)
			{
				fileio_do_movie_save();
				movie_load = 0;
				//cleanup_after_io();
			}

			if (movie_stop)
			{
				movie_stop = 0;
				fileio_do_movie_stop();
			}

			if (schedule_stop)
			{
				NstStopPlaying();
			}
		}
		else
		{
			//gtk_main_iteration_do(TRUE);
		}
	}

	nst_unload();

	fileio_shutdown();
	
	audio_deinit();
	
	input_deinit();
	input_config_write();
	
	config_file_write();

	return 0;
}

void nst_set_framerate() {
	// Set the framerate based on region
	Machine machine(emulator);
	Cartridge::Database database(emulator);

	//framerate = 60;
	if (conf.misc_video_region == 2) {
		machine.SetMode(Machine::PAL);
		//framerate = 50;
	}
	else if (conf.misc_video_region == 1) {
		machine.SetMode(Machine::NTSC);
	}
	else {
		if (database.IsLoaded()) {
			if (dbentry.GetSystem() == Cartridge::Profile::System::NES_PAL) {
				machine.SetMode(Machine::PAL);
				//framerate = 50;
			}
			else {
				machine.SetMode(Machine::NTSC);
			}
		}
		else {
			machine.SetMode(machine.GetDesiredMode());
		}
	}
}

void set_rewinder_direction(int direction) {
	// Set the rewinder backward or forward
	switch (direction) {
		case 0:
			Rewinder(emulator).SetDirection(Rewinder::BACKWARD);
			break;
			
		case 1:
			Rewinder(emulator).SetDirection(Rewinder::FORWARD);
			break;
			
		default: break;
	}
}

// initialize input going into the game
void SetupInput()
{
	// connect a standard NES pad onto the first port
	//Input(emulator).ConnectController( 0, Input::PAD1 );
	
	// connect a standard NES pad onto the second port too
	//Input(emulator).ConnectController( 1, Input::PAD2 );
	
	// connect the Zapper to port 2
	//Input(emulator).ConnectController( 1, Input::ZAPPER );
	
	Input(emulator).AutoSelectController(0);
	Input(emulator).AutoSelectController(1);
	
	// Use the crosshair if a Zapper is present
	if (Input(emulator).GetConnectedController(0) == 5 ||
		Input(emulator).GetConnectedController(1) == 5) {
		cursor_set_crosshair();
	}
}

void configure_savename(const char* filename) {
	
	int i = 0;
	
	// Set up the save directory
	snprintf(savedir, sizeof(savedir), "%ssave/", nstdir);
	
	// Copy the full file path to the savename variable
	snprintf(savename, sizeof(savename), "%s", filename);
	
	// strip the . and extention off the filename for saving
	for (i = strlen(savename)-1; i > 0; i--) {
		if (savename[i] == '.') {
			savename[i] = '\0';
			break;
		}
	}
	
	// Get the name of the game minus file path and extension
	snprintf(gamebasename, sizeof(gamebasename), "%s", basename(savename));
	
	// Construct save path
	snprintf(savename, sizeof(savename), "%s%s%s", savedir, gamebasename, ".sav");

	// Construct root path for FDS save patches
	snprintf(rootname, sizeof(rootname), "%s%s", savedir, gamebasename);
}

// try and find a patch for the game being loaded
static int find_patch(char *patchname)
{
	FILE *f;

	// did the user turn off auto softpatching?
	if (!conf.misc_soft_patching)
	{
		return 0;
	}

	snprintf(patchname, 511, "%s.ips", gamebasename);
	if ((f = fopen(patchname, "rb")) != NULL)
	{
		fclose(f);
		return 1;
	}
	else
	{
		snprintf(patchname, 511, "%s.ups", gamebasename);
		if ((f = fopen(patchname, "rb")) != NULL)
		{
			fclose(f);
			return 1;
		}
	}

	return 0;
}

// load a game or NES music file
void NstLoadGame(const char* filename)
{
	// acquire interface to machine
	Cartridge::Database database( emulator );
	Machine machine( emulator );
	Nsf nsf( emulator );
	Nes::Result result;
	unsigned char *compbuffer;
	int compsize, wascomp, compoffset;
	char gamename[512], patchname[512];

	if (nsf_mode)
	{
		Nsf nsf( emulator );

		nsf.StopSong();

		playing = 0;
	}

	// unload if necessary
	nst_unload();

	// (re)configure savename
	configure_savename(filename);

	// check if it's an archive
	wascomp = 0;
	if (fileio_load_archive(filename, &compbuffer, &compsize, &compoffset, NULL, gamename))
	{
		std::istrstream file((char *)compbuffer+compoffset, compsize);
		wascomp = 1;

		strncpy(lastarchname, filename, sizeof(lastarchname));
	
		configure_savename(gamename);

		if (database.IsLoaded())
		{
			dbentry = database.FindEntry((void *)&compbuffer[compoffset], compsize, get_favored_system());
		}

		if (find_patch(patchname))
		{
			std::ifstream pfile(patchname, std::ios::in|std::ios::binary);

			Machine::Patch patch(pfile, false);

			// load game and softpatch
			result = machine.Load( file, get_favored_system(), patch );
		}
		else
		{
			// load game
			result = machine.Load( file, get_favored_system() );
		}
	}
	else
	{
		FILE *f;
		int length;
		unsigned char *buffer;

		// this is a little ugly
		if (database.IsLoaded())
		{
			f = fopen(filename, "rb");
			if (!f)
			{
				loaded = 0;
				return;
			}

			fseek(f, 0, SEEK_END);
			length = ftell(f);
			fseek(f, 0, SEEK_SET);

			buffer = (unsigned char *)malloc(length);
			fread(buffer, length, 1, f);
			fclose(f);

			dbentry = database.FindEntry(buffer, length, get_favored_system());

			free(buffer);
		}
	
		configure_savename(filename);

		// C++ file stream
		std::ifstream file(filename, std::ios::in|std::ios::binary);

		if (find_patch(patchname))
		{
			std::ifstream pfile(patchname, std::ios::in|std::ios::binary);

			Machine::Patch patch(pfile, false);

			// load game and softpatch
			result = machine.Load( file, get_favored_system(), patch );
		}
		else
		{
			// load game
			result = machine.Load( file, get_favored_system() );
		}
	}

	// failed?
	if (NES_FAILED(result))
	{
		switch (result)
		{
			case Nes::RESULT_ERR_INVALID_FILE:
				std::cout << "Invalid file\n";
				break;

			case Nes::RESULT_ERR_OUT_OF_MEMORY:
				std::cout << "Out of memory\n";
				break;

			case Nes::RESULT_ERR_CORRUPT_FILE:
				std::cout << "Corrupt or missing file\n";
				break;

			case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:
				std::cout << "Unsupported mapper\n";
				break;

			case Nes::RESULT_ERR_MISSING_BIOS:
				snprintf(msgbuf, sizeof(msgbuf), "FDS games require the FDS BIOS.\nIt should be located at ~/.nestopia/disksys.rom");
				//create_messagewindow(msgbuf);
				break;

			default:
				std::cout << "Unknown error #" << result << "\n";
				break;
		}

		return;
	}

	// free the buffer if necessary
	if (wascomp)
	{
		free(compbuffer);
	}

	else
	{
		if (machine.Is(Machine::DISK))
		{
			Fds fds( emulator );

			fds.InsertDisk(0, 0);
			print_fds_info();
		}
	}

	// note that something is loaded
	loaded = 1;

	// power on
	machine.Power( true ); // false = power off
}
