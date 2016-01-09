/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
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
#ifndef _MINGW
#include <archive.h>
#include <archive_entry.h>
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
#include "core/api/NstApiNsf.hpp"

#include "main.h"
#include "cli.h"
#include "audio.h"
#include "video.h"
#include "input.h"
#include "config.h"
#include "cheats.h"

#ifdef _GTK
#include "gtkui/gtkui.h"
#include "gtkui/gtkui_archive.h"
#endif

using namespace Nes::Api;

// base class, all interfaces derives from this
Emulator emulator;

bool loaded = false;
bool playing = false;
bool updateok = false;

bool nst_pal = false;
bool nst_nsf = false;

static int nst_quit = 0;

nstpaths_t nstpaths;

static Video::Output *cNstVideo;
static Sound::Output *cNstSound;
static Input::Controllers *cNstPads;
static Cartridge::Database::Entry dbentry;

static std::ifstream *nstdb;
static std::ifstream *fdsbios;

static std::ifstream *moviefile;
static std::fstream *movierecfile;

extern settings_t conf;
extern bool altspeed;

extern int drawtext;
extern char textbuf[32];

extern bool drawtime;
extern char timebuf[6];

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
			snprintf(timebuf, sizeof(timebuf), "%s", (const char*)data + strlen((char*)data) - 5);
			drawtime = true;
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
	fprintf(stderr, "\rEmulation stopped\n");
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
	video_set_cursor();
}

void nst_fds_info() {
	Fds fds(emulator);

	char* disk;
	char* side;

	fds.GetCurrentDisk() == 0 ? disk = "1" : disk = "2";
	fds.GetCurrentDiskSide() == 0 ? side = "A" : side = "B";

	fprintf(stderr, "Fds: Disk %s Side %s\n", disk, side);
	snprintf(textbuf, sizeof(textbuf), "Disk %s Side %s", disk, side); drawtext = 120;
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
	fprintf(stderr, "State Saved: %s\n", filename);
	snprintf(textbuf, sizeof(textbuf), "State Saved."); drawtext = 120;
}

void nst_state_load(char *filename) {
	// Load a state by filename
	Machine machine(emulator);
	
	std::ifstream statefile(filename, std::ifstream::in|std::ifstream::binary);
	
	if (statefile.is_open()) { machine.LoadState(statefile); }
	fprintf(stderr, "State Loaded: %s\n", filename);
	snprintf(textbuf, sizeof(textbuf), "State Loaded."); drawtext = 120; 
}

void nst_state_quicksave(int slot) {
	// Quick Save State
	char slotpath[520];
	snprintf(slotpath, sizeof(slotpath), "%s_%d.nst", nstpaths.statepath, slot);
	nst_state_save(slotpath);
}


void nst_state_quickload(int slot) {
	// Quick Load State
	char slotpath[520];
	snprintf(slotpath, sizeof(slotpath), "%s_%d.nst", nstpaths.statepath, slot);
		
	struct stat qloadstat;
	if (stat(slotpath, &qloadstat) == -1) {
		fprintf(stderr, "No State to Load\n"); drawtext = 120;
		snprintf(textbuf, sizeof(textbuf), "No State to Load.");
		return;
	}
	
	nst_state_load(slotpath);
}

void nst_movie_save(char *filename) {
	// Save/Record a movie
	Movie movie(emulator);
	
	movierecfile = new std::fstream(filename, std::ifstream::out|std::ifstream::binary); 

	if (movierecfile->is_open()) {
		movie.Record((std::iostream&)*movierecfile, Nes::Api::Movie::CLEAN);
	}
	else {
		delete movierecfile;
		movierecfile = NULL;
	}
}

void nst_movie_load(char *filename) {
	// Load and play a movie
	Movie movie(emulator);
	
	moviefile = new std::ifstream(filename, std::ifstream::in|std::ifstream::binary); 

	if (moviefile->is_open()) {
		movie.Play(*moviefile);
	}
	else {
		delete moviefile;
		moviefile = NULL;
	}
}

void nst_movie_stop() {
	// Stop any movie that is playing or recording
	Movie movie(emulator);
	
	if (movie.IsPlaying() || movie.IsRecording()) {
		movie.Stop();
		movierecfile = NULL;
		delete movierecfile;
		moviefile = NULL;
		delete moviefile;
	}
}

void nst_play() {
	// Play the game
	if (playing || !loaded) { return; }
	
	video_init();
	audio_init();
	input_init();
	cheats_init();
	
	cNstVideo = new Video::Output;
	cNstSound = new Sound::Output;
	cNstPads  = new Input::Controllers;
	
	audio_set_params(cNstSound);
	audio_unpause();
	
	if (nst_nsf) {
		Nsf nsf(emulator);
		nsf.PlaySong();
		video_disp_nsf();
	}
	
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
	
	// create screenshots directory if it doesn't exist
	snprintf(dirstr, sizeof(dirstr), "%sscreenshots", nstpaths.nstdir);
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
	
	// Construct the save state path
	snprintf(nstpaths.statepath, sizeof(nstpaths.statepath), "%sstate/%s", nstpaths.nstdir, nstpaths.gamename);
	
	// Construct the cheat path
	snprintf(nstpaths.cheatpath, sizeof(nstpaths.cheatpath), "%scheats/%s.xml", nstpaths.nstdir, nstpaths.gamename);
}

bool nst_archive_checkext(const char *filename) {
	// Check if the file extension is valid
	int len = strlen(filename);

	if ((!strcasecmp(&filename[len-4], ".nes")) ||
	    (!strcasecmp(&filename[len-4], ".fds")) ||
	    (!strcasecmp(&filename[len-4], ".nsf")) ||
	    (!strcasecmp(&filename[len-4], ".unf")) ||
	    (!strcasecmp(&filename[len-5], ".unif"))||
	    (!strcasecmp(&filename[len-4], ".xml"))) {
		return true;
	}
	return false;
}

bool nst_archive_handle(const char *filename, char **rom, int *romsize, const char *reqfile) {
	// Handle archives
#ifndef _MINGW
	struct archive *a;
	struct archive_entry *entry;
	int r;
	int64_t entrysize;
	
	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, filename, 10240);
	
	// Test if it's actually an archive
	if (r != ARCHIVE_OK) {
		r = archive_read_free(a);
		return false;
	}
	
	// Scan through the archive for files
	while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
		char *rombuf;
		const char *currentfile = archive_entry_pathname(entry);
		
		if (nst_archive_checkext(currentfile)) {
			nst_set_paths(currentfile);
			
			// If there's a specific file we want, load it
			if (reqfile != NULL) {
				if (!strcmp(currentfile, reqfile)) {
					entrysize = archive_entry_size(entry);
					rombuf = (char*)malloc(entrysize);
					archive_read_data(a, rombuf, entrysize);
					archive_read_data_skip(a);
					r = archive_read_free(a);
					*romsize = entrysize;
					*rom = rombuf;
					return true;
				}
			}
			// Otherwise just take the first file in the archive
			else {
				entrysize = archive_entry_size(entry);
				rombuf = (char*)malloc(entrysize);
				archive_read_data(a, rombuf, entrysize);
				archive_read_data_skip(a);
				r = archive_read_free(a);
				*romsize = entrysize;
				*rom = rombuf;
				return true;
			}
		}
	}
#endif
	return false;
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
	Nsf nsf(emulator);
	Sound sound(emulator);
	Nes::Result result;
	char *rom;
	int romsize;
	char patchname[512];
	
	// Pause play before pulling out a cartridge
	if (playing) { nst_pause(); }
	
	// Pull out any inserted cartridges
	nst_unload(); drawtime = false;
	
	// Handle the file as an archive if it is one
	#ifdef _GTK
	char reqname[256];
	bool isarchive = gtkui_archive_handle(filename, reqname, sizeof(reqname));
	
	if (isarchive) {
		nst_archive_handle(filename, &rom, &romsize, reqname);
	#else
	if (nst_archive_handle(filename, &rom, &romsize, NULL)) {
	#endif
		// Convert the malloc'd char* to an istream
		std::string rombuf(rom, romsize);
		std::istringstream file(rombuf);
		result = machine.Load(file, nst_default_system());
	}
	// Otherwise just load the file
	else {
		std::ifstream file(filename, std::ios::in|std::ios::binary);
		
		// Set the file paths
		nst_set_paths(filename);
		
		if (nst_find_patch(patchname)) { // Load with a patch if there is one
			std::ifstream pfile(patchname, std::ios::in|std::ios::binary);
			Machine::Patch patch(pfile, false);
			result = machine.Load(file, nst_default_system(), patch);
		}
		else { result = machine.Load(file, nst_default_system()); }
	}
	
	if (NES_FAILED(result)) {
		char errorstring[32];
		#ifdef _GTK
		if (conf.video_fullscreen) { video_toggle_fullscreen(); }
		#endif
		switch (result) {
			case Nes::RESULT_ERR_INVALID_FILE:
				snprintf(errorstring, sizeof(errorstring), "Error: Invalid file");
				break;

			case Nes::RESULT_ERR_OUT_OF_MEMORY:
				snprintf(errorstring, sizeof(errorstring), "Error: Out of Memory");
				break;

			case Nes::RESULT_ERR_CORRUPT_FILE:
				snprintf(errorstring, sizeof(errorstring), "Error: Corrupt or Missing File");
				break;

			case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:
				snprintf(errorstring, sizeof(errorstring), "Error: Unsupported Mapper");
				break;

			case Nes::RESULT_ERR_MISSING_BIOS:
				snprintf(errorstring, sizeof(errorstring), "Error: Missing Fds BIOS");
				break;

			default:
				snprintf(errorstring, sizeof(errorstring), "Error: %d", result);
				break;
		}
		
		fprintf(stderr, "%s\n", errorstring);
		#ifdef _GTK
		if (conf.misc_disable_gui) { cli_error(errorstring); }
		else {
			if (conf.video_fullscreen) {
				video_destroy();
				conf.video_fullscreen = false;
				video_create();
				video_set_cursor();
			}
			gtkui_message(errorstring);
		}
		#endif
		
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
	
	// Check if this is an NSF
	nst_nsf = (machine.Is(Machine::SOUND));
	if (nst_nsf) { nsf.StopSong(); }
	
	// Check if sound distortion should be enabled
	sound.SetGenie(conf.misc_genie_distortion);
	
	// note that something is loaded
	loaded = 1;
	
	// Set the title
	video_set_title(nstpaths.gamename);
	#ifdef _GTK
	if (!conf.misc_disable_gui) { gtkui_set_title(nstpaths.gamename); }
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
	
	// Detect Joysticks
	input_joysticks_detect();
	
	// Set default input keys
	input_set_default();
	
	// Read the input config file and override defaults
	input_config_read();
	
	// Set the video dimensions
	video_set_dimensions();
	
	// Create the window
	#ifdef _GTK
	if (!conf.misc_disable_gui) { gtkui_init(argc, argv); }
	#endif
	video_create();
	
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
		#ifdef _GTK // This is a dirty hack
		if (conf.misc_disable_gui) {
			nst_load(argv[argc - 1]);
			if (!loaded) {
				fprintf(stderr, "Fatal: Could not load ROM\n");
				exit(1);
			}
		}
		else {
			if (strcmp(argv[argc - 1], "-e")) { nst_load(argv[argc - 1]); }
		}
		#else
		conf.misc_disable_gui = true;
		nst_load(argv[argc - 1]);
		if (!loaded) {
			fprintf(stderr, "Fatal: Could not load ROM\n");
			exit(1);
		}
		#endif
	}
	
	// Start the main loop
	nst_quit = 0;
	
	while (!nst_quit) {
		#if defined(_APPLE) && defined(_GTK)
		if (!playing) { gtk_main_iteration_do(TRUE); }
		#elif _GTK
		while (gtk_events_pending()) {
			gtk_main_iteration_do(TRUE);
		}
		if (!playing) { gtk_main_iteration_do(TRUE); }
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
				
				// Execute a frame
				if (timing_frameskip()) {
					emulator.Execute(NULL, cNstSound, cNstPads);
				}
				else { emulator.Execute(cNstVideo, cNstSound, cNstPads); }
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
	
	// Deinitialize joysticks
	input_joysticks_close();
	
	// Write the input config file
	input_config_write();
	
	// Write the config file
	config_file_write();

	return 0;
}
