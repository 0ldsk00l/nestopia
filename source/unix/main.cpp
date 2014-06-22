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
#include <sstream>
#include <iomanip>
#include <vector>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <libgen.h>
#ifdef _MINGW
#include <io.h>
#endif

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiSound.hpp"
#include "core/api/NstApiInput.hpp"
#include "core/api/NstApiMachine.hpp"
#include "core/api/NstApiUser.hpp"
#include "core/api/NstApiFds.hpp"
#include "core/api/NstApiDipSwitches.hpp"
#include "core/api/NstApiRewinder.hpp"
#include "core/api/NstApiCartridge.hpp"
#include "core/api/NstApiMovie.hpp"

#include "main.h"
#include "cli.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "config.h"
#include "cheats.h"
#include "cursor.h"

#ifdef _GTK
#include "gtkui/gtkui.h"
#endif

using namespace Nes::Api;

// base class, all interfaces derives from this
Emulator emulator;

bool loaded = false;
bool playing = false;
bool updateok = false;
bool frameskip = false;

bool nst_pal = false;

static int nst_quit = 0;

nstpaths_t nstpaths;

static Video::Output *cNstVideo;
static Sound::Output *cNstSound;
static Input::Controllers *cNstPads;
static Cartridge::Database::Entry dbentry;

static std::ifstream *nstdb;
static std::ifstream *fdsbios;

static std::ifstream *moviefile;

extern settings_t conf;
extern bool altspeed;

// *******************
// emulation callbacks
// *******************

// called right before Nestopia is about to write pixels
static bool NST_CALLBACK VideoLock(void* userData, Video::Output& video) {
	video.pitch = video_lock_screen(video.pixels);
	return true; // true=lock success, false=lock failed (Nestopia will carry on but skip video)
}

// called right after Nestopia has finished writing pixels (not called if previous lock failed)
static void NST_CALLBACK VideoUnlock(void* userData, Video::Output& video) {
	video_unlock_screen(video.pixels);
}

static bool NST_CALLBACK SoundLock(void* userData, Sound::Output& sound) {
	return true;
}

static void NST_CALLBACK SoundUnlock(void* userData, Sound::Output& sound) {
	// Do Nothing
}

static void NST_CALLBACK nst_cb_event(void *userData, User::Event event, const void* data) {
	// Handle special events
	switch (event) {
		case User::EVENT_CPU_JAM:
			fprintf(stderr, "Cpu: Jammed\n");
			break;
		case User::EVENT_CPU_UNOFFICIAL_OPCODE:
			fprintf(stderr, "Cpu: Unofficial Opcode %s\n", (const char*)data);
			break;
		case User::EVENT_DISPLAY_TIMER:
			fprintf(stderr, "\r%s", (const char*)data);
			break;
		default: break;
	}
}

static void NST_CALLBACK nst_cb_log(void *userData, const char *string, unsigned long int length) {
	// Print logging information to stderr
	fprintf(stderr, "%s", string);
}

static void NST_CALLBACK nst_cb_file(void *userData, User::File& file) {
	unsigned char *compbuffer;
	int compsize, compoffset;
	char *filename;
	
	switch (file.GetAction()) {
		case User::File::LOAD_ROM:
			// Nothing here for now			
			break;

		case User::File::LOAD_SAMPLE:
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU:
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88:
		case User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS:
		case User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU:
		case User::File::LOAD_SAMPLE_AEROBICS_STUDIO:
			// Nothing here for now
			break;

		case User::File::LOAD_BATTERY: // load in battery data from a file
		case User::File::LOAD_EEPROM: // used by some Bandai games, can be treated the same as battery files
		case User::File::LOAD_TAPE: // for loading Famicom cassette tapes
		case User::File::LOAD_TURBOFILE: // for loading turbofile data
		{		
			std::ifstream batteryFile(nstpaths.savename, std::ifstream::in|std::ifstream::binary);
			
			if (batteryFile.is_open()) { file.SetContent(batteryFile); }
			break;
		}
		
		case User::File::SAVE_BATTERY: // save battery data to a file
		case User::File::SAVE_EEPROM: // can be treated the same as battery files
		case User::File::SAVE_TAPE: // for saving Famicom cassette tapes
		case User::File::SAVE_TURBOFILE: // for saving turbofile data
		{
			std::ofstream batteryFile(nstpaths.savename, std::ifstream::out|std::ifstream::binary);
			const void* savedata;
			unsigned long savedatasize;

			file.GetContent(savedata, savedatasize);

			if (batteryFile.is_open()) { batteryFile.write((const char*) savedata, savedatasize); }

			break;
		}

		case User::File::LOAD_FDS: // for loading modified Famicom Disk System files
		{
			char fdsname[512];

			snprintf(fdsname, sizeof(fdsname), "%s.ups", nstpaths.fdssave);
			
			std::ifstream batteryFile( fdsname, std::ifstream::in|std::ifstream::binary );

			// no ups, look for ips
			if (!batteryFile.is_open())
			{
				snprintf(fdsname, sizeof(fdsname), "%s.ips", nstpaths.fdssave);

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

			snprintf(fdsname, sizeof(fdsname), "%s.ups", nstpaths.fdssave);

			std::ofstream fdsFile( fdsname, std::ifstream::out|std::ifstream::binary );

			if (fdsFile.is_open())
				file.GetPatchContent( User::File::PATCH_UPS, fdsFile );

			break;
		}
	}
}

static void nst_unload() {
	// Remove the cartridge and shut down the NES
	Machine machine(emulator);
	
	if (!loaded) { return; }
	
	// Power down the NES
	fprintf(stderr, "\rPowering down the emulated machine\n");
	machine.Power(false);

	// Remove the cartridge
	machine.Unload();
}

void nst_pause() {
	// Pauses the game
	if (playing) {
		audio_pause();
		audio_deinit();
	}
	
	playing = false;
	cursor_set_default();
	#ifdef _GTK
	if (!conf.misc_disable_gui) { gtkui_cursor_set_default(); }
	#endif
}

// generate the filename for quicksave files
std::string StrQuickSaveFile(int slot) {
	
	std::ostringstream ossFile;
	ossFile << nstpaths.nstdir;
	ossFile << "state";
	
	ossFile << "/" << std::setbase(16) << std::setfill('0') << std::setw(8)
		<< basename(nstpaths.gamename) << std::string("_") << slot << ".nst";
	
	return ossFile.str();
}

void nst_fds_info() {
	Fds fds(emulator);

	char* disk;
	char* side;

	fds.GetCurrentDisk() == 0 ? disk = "1" : disk = "2";
	fds.GetCurrentDiskSide() == 0 ? side = "A" : side = "B";

	fprintf(stderr, "Fds: Disk %s Side %s\n", disk, side);
}

void nst_flip_disk() {
	// Flips the FDS disk
	Fds fds(emulator);

	if (fds.CanChangeDiskSide()) {
		fds.ChangeSide();
		nst_fds_info();
	}
}

void nst_switch_disk() {
	// Switches the FDS disk in multi-disk games
	Fds fds(emulator);
	
	int currentdisk = fds.GetCurrentDisk();
	
	// If it's a multi-disk game, eject and insert the other disk
	if (fds.GetNumDisks() > 1) {
		fds.EjectDisk();
		fds.InsertDisk(!currentdisk, 0);
		nst_fds_info();
	}
}

static Machine::FavoredSystem nst_default_system() {
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

void nst_dipswitch() {
	// Print DIP switch information and call handler
	DipSwitches dipswitches(emulator);
		
	int numdips = dipswitches.NumDips();
	
	if (numdips > 0) {
		for (int i = 0; i < numdips; i++) {
			fprintf(stderr, "%d: %s\n", i, dipswitches.GetDipName(i));
			int numvalues = dipswitches.NumValues(i);
			
			for (int j = 0; j < numvalues; j++) {
				fprintf(stderr, " %d: %s\n", j, dipswitches.GetValueName(i, j));
			}
		}
		dip_handle();
	}
}

void nst_state_save(char *filename) {
	// Save a state by filename
	Machine machine(emulator);
	
	std::ofstream statefile(filename, std::ifstream::out|std::ifstream::binary);
	
	if (statefile.is_open()) { machine.SaveState(statefile, Nes::Api::Machine::NO_COMPRESSION); }
}

void nst_state_load(char *filename) {
	// Load a state by filename
	Machine machine(emulator);
	
	std::ifstream statefile(filename, std::ifstream::in|std::ifstream::binary);
	
	if (statefile.is_open()) { machine.LoadState(statefile); }
}

void nst_state_quicksave(int slot) {
	// Save State
	std::string strFile = StrQuickSaveFile(slot);
	
	Machine machine( emulator );
	std::ofstream os(strFile.c_str());
	
	machine.SaveState(os, Nes::Api::Machine::NO_COMPRESSION);
	fprintf(stderr, "State Saved: %s\n", strFile.c_str());
}


void nst_state_quickload(int slot) {
	// Load State
	std::string strFile = StrQuickSaveFile(slot);
	
	struct stat qloadstat;
	if (stat(strFile.c_str(), &qloadstat) == -1) {
		fprintf(stderr, "No State to Load\n");
		return;
	}

	Machine machine( emulator );
	std::ifstream is(strFile.c_str());
	machine.LoadState(is);
	fprintf(stderr, "State Loaded: %s\n", strFile.c_str());
}

void nst_movie_save(char *filename) {
	// Save/Record a movie
}

void nst_movie_load(char *filename) {
	// Load and play a movie
}

void nst_movie_stop() {
	// Stop any movie that is playing or recording
	Movie movie(emulator);
	
	if (movie.IsPlaying() || movie.IsRecording()) {
		movie.Stop();
		moviefile = NULL;
		delete moviefile;
	}
}

void nst_play() {
	// Play the game
	if (playing || !loaded) { return; }
	
	video_init();
	audio_init();
	SetupInput();
	cheats_init();
	
	cNstVideo = new Video::Output;
	cNstSound = new Sound::Output;
	cNstPads  = new Input::Controllers;
	
	audio_set_params(cNstSound);
	audio_unpause();
	
	audio_set_samples(cNstSound->length[0]);
	
	updateok = false;
	playing = true;
}

void nst_reset(bool hardreset) {
	// Reset the machine (soft or hard)
	Machine machine(emulator);
	Fds fds(emulator);
	machine.Reset(hardreset);
	
	// Set the FDS disk to defaults
	fds.EjectDisk();
	fds.InsertDisk(0, 0);
}

void nst_schedule_quit() {
	nst_quit = 1;
}

void nst_set_dirs() {
	// Set up system directories
#ifdef _MINGW
	snprintf(nstpaths.nstdir, sizeof(nstpaths.nstdir), "");
#else
	// create system directory if it doesn't exist
	snprintf(nstpaths.nstdir, sizeof(nstpaths.nstdir), "%s/.nestopia/", getenv("HOME"));
	if (mkdir(nstpaths.nstdir, 0755) && errno != EEXIST) {	
		fprintf(stderr, "Failed to create %s: %d\n", nstpaths.nstdir, errno);
	}
#endif
	// create save and state directories if they don't exist
	char dirstr[256];
	snprintf(dirstr, sizeof(dirstr), "%ssave", nstpaths.nstdir);
#ifdef _MINGW	
	if (mkdir(dirstr) && errno != EEXIST) {
#else
	if (mkdir(dirstr, 0755) && errno != EEXIST) {
#endif
		fprintf(stderr, "Failed to create %s: %d\n", dirstr, errno);
	}

	snprintf(dirstr, sizeof(dirstr), "%sstate", nstpaths.nstdir);
#ifdef _MINGW	
	if (mkdir(dirstr) && errno != EEXIST) {
#else
	if (mkdir(dirstr, 0755) && errno != EEXIST) {
#endif
		fprintf(stderr, "Failed to create %s: %d\n", dirstr, errno);
	}
	
	// create cheats directory if it doesn't exist
	snprintf(dirstr, sizeof(dirstr), "%scheats", nstpaths.nstdir);
#ifdef _MINGW	
	if (mkdir(dirstr) && errno != EEXIST) {
#else
	if (mkdir(dirstr, 0755) && errno != EEXIST) {
#endif
		fprintf(stderr, "Failed to create %s: %d\n", dirstr, errno);
	}
}

void nst_set_region() {
	// Set the region
	Machine machine(emulator);
	Cartridge::Database database(emulator);
	//Cartridge::Profile profile;
	
	if (database.IsLoaded()) {
		//std::ifstream dbfile(filename, std::ios::in|std::ios::binary);
		//Cartridge::ReadInes(dbfile, nst_default_system(), profile);
		//dbentry = database.FindEntry(profile.hash, nst_default_system());
		
		machine.SetMode(machine.GetDesiredMode());
		
		if (machine.GetMode() == Machine::PAL) {
			fprintf(stderr, "Region: PAL\n");
			nst_pal = true;
		}
		else {
			fprintf(stderr, "Region: NTSC\n");
			nst_pal = false;
		}
		//printf("Mapper: %d\n", dbentry.GetMapper());
	}
}

void nst_set_rewind(int direction) {
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
		#ifdef _GTK
		gtkui_cursor_set_crosshair();
		#endif
	}
}

void nst_set_paths(const char *filename) {
	
	// Set up the save directory
	snprintf(nstpaths.savedir, sizeof(nstpaths.savedir), "%ssave/", nstpaths.nstdir);
	
	// Copy the full file path to the savename variable
	snprintf(nstpaths.savename, sizeof(nstpaths.savename), "%s", filename);
	
	// strip the . and extention off the filename for saving
	for (int i = strlen(nstpaths.savename)-1; i > 0; i--) {
		if (nstpaths.savename[i] == '.') {
			nstpaths.savename[i] = '\0';
			break;
		}
	}
	
	// Get the name of the game minus file path and extension
	snprintf(nstpaths.gamename, sizeof(nstpaths.gamename), "%s", basename(nstpaths.savename));
	
	// Construct save path
	snprintf(nstpaths.savename, sizeof(nstpaths.savename), "%s%s%s", nstpaths.savedir, nstpaths.gamename, ".sav");

	// Construct path for FDS save patches
	snprintf(nstpaths.fdssave, sizeof(nstpaths.fdssave), "%s%s", nstpaths.savedir, nstpaths.gamename);
	
	// Construct the cheat path
	snprintf(nstpaths.cheatpath, sizeof(nstpaths.cheatpath), "%scheats/%s.xml", nstpaths.nstdir, nstpaths.gamename);
}

bool nst_find_patch(char *filename) {
	// Check for a patch in the same directory as the game
	FILE *file;
	
	if (!conf.misc_soft_patching) {
		return 0;
	}
	
	snprintf(filename, sizeof(nstpaths.savename), "%s.ips", nstpaths.gamename);
	
	if ((file = fopen(filename, "rb")) != NULL) {
		fclose(file);
		return 1;
	}
	else {
		snprintf(filename, sizeof(nstpaths.savename), "%s.ups", nstpaths.gamename);
		
		if ((file = fopen(filename, "rb")) != NULL) {
			fclose(file);
			return 1;
		}
	}
	
	return 0;
}

void nst_load_db() {
	Nes::Api::Cartridge::Database database(emulator);
	char dbpath[512];

	if (nstdb) { return; }

	// Try to open the database file
	snprintf(dbpath, sizeof(dbpath), "%sNstDatabase.xml", nstpaths.nstdir);
	nstdb = new std::ifstream(dbpath, std::ifstream::in|std::ifstream::binary);
	
	if (nstdb->is_open()) {
		database.Load(*nstdb);
		database.Enable(true);
		return;
	}
#ifndef _MINGW
	// If it fails, try looking in the data directory
	snprintf(dbpath, sizeof(dbpath), "%s/NstDatabase.xml", DATADIR);
	nstdb = new std::ifstream(dbpath, std::ifstream::in|std::ifstream::binary);
	
	if (nstdb->is_open()) {
		database.Load(*nstdb);
		database.Enable(true);
		return;
	}
	
	// If that fails, try looking in the working directory
	char *pwd = getenv("PWD");
	snprintf(dbpath, sizeof(dbpath), "%s/NstDatabase.xml", pwd);
	nstdb = new std::ifstream(dbpath, std::ifstream::in|std::ifstream::binary);
	
	if (nstdb->is_open()) {
		database.Load(*nstdb);
		database.Enable(true);
		return;
	}
#endif
	else {
		fprintf(stderr, "NstDatabase.xml not found!\n");
		delete nstdb;
		nstdb = NULL;
	}
}

void nst_load_fds_bios() {
	// Load the Famicom Disk System BIOS
	Nes::Api::Fds fds(emulator);
	char biospath[512];
	
	if (fdsbios) { return; }

	snprintf(biospath, sizeof(biospath), "%sdisksys.rom", nstpaths.nstdir);

	fdsbios = new std::ifstream(biospath, std::ifstream::in|std::ifstream::binary);

	if (fdsbios->is_open())	{
		fds.SetBIOS(fdsbios);
	}
	else {
		fprintf(stderr, "%s not found, Disk System games will not work.\n", biospath);
		delete fdsbios;
		fdsbios = NULL;
	}
}

void nst_load(const char *filename) {
	// Load a Game ROM
	Machine machine(emulator);
	Sound sound(emulator);
	Nes::Result result;
	char patchname[512];
	
	// Pause play before pulling out a cartridge
	if (playing) { nst_pause(); }
	
	// Pull out any inserted cartridges
	nst_unload();

	// Set the file paths
	nst_set_paths(filename);

	// C++ file stream
	std::ifstream file(filename, std::ios::in|std::ios::binary);

	if (nst_find_patch(patchname)) {
		std::ifstream pfile(patchname, std::ios::in|std::ios::binary);

		Machine::Patch patch(pfile, false);

		// Soft Patch
		result = machine.Load(file, nst_default_system(), patch);
	}
	else {
		result = machine.Load(file, nst_default_system());
	}
	
	if (NES_FAILED(result)) {
		switch (result) {
			case Nes::RESULT_ERR_INVALID_FILE:
				fprintf(stderr, "Error: Invalid file\n");
				break;

			case Nes::RESULT_ERR_OUT_OF_MEMORY:
				fprintf(stderr, "Error: Out of Memory\n");
				break;

			case Nes::RESULT_ERR_CORRUPT_FILE:
				fprintf(stderr, "Error: Corrupt or Missing File\n");
				break;

			case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:
				fprintf(stderr, "Error: Unsupported Mapper\n");
				break;

			case Nes::RESULT_ERR_MISSING_BIOS:
				fprintf(stderr, "Error: Missing Fds BIOS\n");
				break;

			default:
				fprintf(stderr, "Error: %d\n", result);
				break;
		}
		return;
	}
	
	// Deal with any DIP Switches
	nst_dipswitch();
	
	// Set the region
	nst_set_region();
	
	if (machine.Is(Machine::DISK)) {
		Fds fds(emulator);
		fds.InsertDisk(0, 0);
		nst_fds_info();
	}
	
	// Check if sound distortion should be enabled
	sound.SetGenie(conf.misc_genie_distortion);
	
	// note that something is loaded
	loaded = 1;
	
	// Set the title
	video_set_title(nstpaths.gamename);
	#ifdef _GTK
	if (!conf.misc_disable_gui) { gtkui_opengl_start(); gtkui_set_title(nstpaths.gamename); }
	#endif

	// power on
	machine.Power(true); // false = power off
	
	nst_play();
}

int main(int argc, char *argv[]) {
	// This is the main function
	
	static SDL_Event event;
	void *userData = (void*)0xDEADC0DE;

	// Set up directories
	nst_set_dirs();
	
	// Set default config options
	config_set_default();
	
	// Read the config file and override defaults
	config_file_read();
	
	// Exit if there is no CLI argument
	#ifdef _GTK
	if (argc == 1 && conf.misc_disable_gui) {
	#else
	if (argc == 1) {
	#endif
		cli_show_usage();
		return 0;
	}
	
	// Handle command line arguments
	cli_handle_command(argc, argv);
	
	// Initialize SDL
	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_JOYSTICK) < 0) {
		fprintf(stderr, "Couldn't initialize SDL: %s\n", SDL_GetError());
		return 1;
	}
	
	// Initialize input
	input_init();
	
	// Set default input keys
	input_set_default();
	
	// Read the input config file and override defaults
	input_config_read();
	
	// Set up the video parameters
	video_set_params();
	
	// Create the window
	#ifdef _GTK
	conf.misc_disable_gui ? video_create() : gtkui_init(argc, argv);
	if (conf.video_fullscreen) { video_create(); }
	#else
	video_create();
	#endif
	
	// Set up the callbacks
	Video::Output::lockCallback.Set(VideoLock, userData);
	Video::Output::unlockCallback.Set(VideoUnlock, userData);
	
	Sound::Output::lockCallback.Set(SoundLock, userData);
	Sound::Output::unlockCallback.Set(SoundUnlock, userData);
	
	User::fileIoCallback.Set(nst_cb_file, userData);
	User::logCallback.Set(nst_cb_log, userData);
	User::eventCallback.Set(nst_cb_event, userData);
	
	// Initialize and load FDS BIOS and NstDatabase.xml
	nstdb = NULL;
	fdsbios = NULL;
	nst_load_db();
	nst_load_fds_bios();

	// Load a rom from the command line
	if (argc > 1) {
		nst_load(argv[argc - 1]);
		
		if (!loaded) {
			fprintf(stderr, "Fatal: Could not load ROM\n");
			exit(1);
		}
	}
	
	// Start the main loop
	nst_quit = 0;
	
	while (!nst_quit) {
		#ifdef _GTK
		while (gtk_events_pending()) {
			gtk_main_iteration_do(FALSE);
		}
		#endif
		if (playing) {
			while (SDL_PollEvent(&event)) {
				switch (event.type) {
					case SDL_QUIT:
						nst_quit = 1;
						break;
					
					case SDL_KEYDOWN:
					case SDL_KEYUP:
					case SDL_JOYHATMOTION:
					case SDL_JOYAXISMOTION:
					case SDL_JOYBUTTONDOWN:
					case SDL_JOYBUTTONUP:
					case SDL_MOUSEBUTTONDOWN:
					case SDL_MOUSEBUTTONUP:
						input_process(cNstPads, event);
						break;
					default: break;
				}	
			}
			
			if (NES_SUCCEEDED(Rewinder(emulator).Enable(true))) {
				Rewinder(emulator).EnableSound(true);
			}
			
			audio_play();
			
			if (updateok) {
				// Pulse the turbo buttons
				input_pulse_turbo(cNstPads);
				
				// Check if it's time to skip a frame
				frameskip = timing_frameskip();
				
				// Execute a frame
				if (!frameskip) {
					emulator.Execute(cNstVideo, cNstSound, cNstPads);
				}
				else { emulator.Execute(NULL, cNstSound, cNstPads); }
				
				// Prevent insane speeds when vsync is turned off...
				if (!altspeed) { updateok = false; }
				
				// ...but only use the limiter when enabled
				if (!conf.timing_limiter) { updateok = true; }
			}
		}
	}
	
	// Remove the cartridge and shut down the NES
	nst_unload();
	
	// Unload the FDS BIOS and NstDatabase.xml
	if (nstdb) { delete nstdb; nstdb = NULL; }
	if (fdsbios) { delete fdsbios; fdsbios = NULL; }
	
	// Deinitialize audio
	audio_deinit();
	
	// Deinitialize input
	input_deinit();
	
	// Write the input config file
	input_config_write();
	
	// Write the config file
	config_file_write();

	return 0;
}
