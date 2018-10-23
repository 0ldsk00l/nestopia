/*
 * Nestopia UE
 * 
 * Copyright (C) 2007-2008 R. Belmont
 * Copyright (C) 2012-2018 R. Danbrook
 * Copyright (C) 2018-2018 Phil Smith
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <libgen.h>

#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>

#ifdef _MINGW
#include <io.h>
#endif
#ifndef _MINGW
#include <archive.h>
#include <archive_entry.h>
#endif

// Nst Common
#include "nstcommon.h"
#include "config.h"
#include "cheats.h"
#include "homebrew.h"
#include "input.h"
#include "audio.h"
#include "video.h"
#include "samples.h"

Emulator emulator;
Video::Output *cNstVideo;
Sound::Output *cNstSound;
Input::Controllers *cNstPads;

nstpaths_t nstpaths;

static bool ffspeed = false;
static bool playing = false;

static std::ifstream *nstdb;

static std::ifstream *fdsbios;

static std::ifstream *moviefile;
static std::fstream *movierecfile;

void *custompalette = NULL;
static size_t custpalsize;

bool (*nst_archive_select)(const char*, char*, size_t);

static bool NST_CALLBACK nst_cb_videolock(void* userData, Video::Output& video) {
	video.pitch = video_lock_screen(video.pixels);
	return true; // true=lock success, false=lock failed (Nestopia will carry on but skip video)
}

static void NST_CALLBACK nst_cb_videounlock(void* userData, Video::Output& video) {
	video_unlock_screen(video.pixels);
}

static bool NST_CALLBACK nst_cb_soundlock(void* userData, Sound::Output& sound) {
	return true;
}

static void NST_CALLBACK nst_cb_soundunlock(void* userData, Sound::Output& sound) {
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
			nst_video_print_time((const char*)data + strlen((char*)data) - 5, true);
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

		case User::File::LOAD_SAMPLE: break;
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU: nst_sample_load_samples(file, "moepro"); break;
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88: nst_sample_load_samples(file, "moepro88"); break;
		case User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS: nst_sample_load_samples(file, "mptennis"); break;
		case User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU: nst_sample_load_samples(file, "terao"); break;
		case User::File::LOAD_SAMPLE_AEROBICS_STUDIO: nst_sample_load_samples(file, "ftaerobi"); break;

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

static Machine::FavoredSystem nst_default_system() {
	switch (conf.misc_default_system) {
		case 2: return Machine::FAVORED_NES_PAL; break;
		case 3: return Machine::FAVORED_FAMICOM; break;
		case 4: return Machine::FAVORED_DENDY; break;
		default: return Machine::FAVORED_NES_NTSC; break;
	}
}

void* nst_ptr_video() { return &cNstVideo; }
void* nst_ptr_sound() { return &cNstSound; }
void* nst_ptr_input() { return &cNstPads; }

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

bool nst_archive_select_file(const char *filename, char *reqfile, size_t reqsize) {
	// Select a filename to pull out of the archive
#ifndef _MINGW
	struct archive *a;
	struct archive_entry *entry;
	int r, numarchives = 0;
	
	a = archive_read_new();
	archive_read_support_filter_all(a);
	archive_read_support_format_all(a);
	r = archive_read_open_filename(a, filename, 10240);
	
	// Test if it's actually an archive
	if (r != ARCHIVE_OK) {
		r = archive_read_free(a);
		return false;
	}
	// If it is an archive, handle it
	else {
		// Find files with valid extensions within the archive
		while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
			const char *currentfile = archive_entry_pathname(entry);
			if (nst_archive_checkext(currentfile)) {
				numarchives++;
				snprintf(reqfile, reqsize, "%s", currentfile);
			}
			archive_read_data_skip(a);
			break; // Load the first one found
		}
		// Free the archive
		r = archive_read_free(a);
		
		// If there are no valid files in the archive, return
		if (numarchives == 0) {	return false; }
		else { return true; }
	}
#endif
	return false;
}

bool nst_archive_open(const char *filename, char **rom, int *romsize, const char *reqfile) {
	// Opens archives
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

void nst_db_load() {
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

void nst_db_unload() {
	if (nstdb) { delete nstdb; nstdb = NULL; }
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
		
		char dippath[512];
		snprintf(dippath, sizeof(dippath), "%s%s.dip", nstpaths.savedir, nstpaths.gamename);
		nst_dip_handle(dippath);
	}
}

void nst_fds_bios_load() {
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
		fprintf(stderr, "Fds: BIOS not found: %s\n", biospath);
		delete fdsbios;
		fdsbios = NULL;
	}
}

void nst_fds_bios_unload() {
	if (fdsbios) { delete fdsbios; fdsbios = NULL; }
}

void nst_fds_info() {
	Fds fds(emulator);

	const char* disk;
	const char* side;
	char textbuf[24];
	
	fds.GetCurrentDisk() == 0 ? disk = "1" : disk = "2";
	fds.GetCurrentDiskSide() == 0 ? side = "A" : side = "B";

	fprintf(stderr, "Fds: Disk %s Side %s\n", disk, side);
	snprintf(textbuf, sizeof(textbuf), "Disk %s Side %s", disk, side);
	nst_video_print((const char*)textbuf, 8, 16, 2, true);
}

void nst_fds_flip() {
	// Flips the FDS disk
	Fds fds(emulator);

	if (fds.CanChangeDiskSide()) {
		fds.ChangeSide();
		nst_fds_info();
	}
}

void nst_fds_switch() {
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

bool nst_nsf() {
	Machine machine(emulator);
	return machine.Is(Machine::SOUND);
}

void nst_nsf_play() {
	Nsf nsf(emulator);
	nsf.PlaySong();
	video_clear_buffer();
	video_disp_nsf();
}

void nst_nsf_stop() {
	Nsf nsf(emulator);
	nsf.StopSong();
}

void nst_nsf_prev() {
	Nsf nsf(emulator);
	nsf.SelectPrevSong();
	video_clear_buffer();
	video_disp_nsf();
}

void nst_nsf_next() {
	Nsf nsf(emulator);
	nsf.SelectNextSong();
	video_clear_buffer();
	video_disp_nsf();
}

bool nst_pal() {
	Machine machine(emulator);
	return machine.GetMode() == Machine::PAL;
}

bool nst_playing() { return playing; }

void nst_palette_load(const char *filename) {
	// Load a custom palette
	
	FILE *file;
	long filesize; // File size in bytes
	size_t result;
	
	char custgamepalpath[512];
	snprintf(custgamepalpath, sizeof(custgamepalpath), "%s%s%s", nstpaths.nstdir, nstpaths.gamename, ".pal");
	
	// Try the game-specific palette first
	file = fopen(custgamepalpath, "rb");
	if (!file) { file = fopen(filename, "rb"); }
	
	// Then try the global custom palette
	if (!file) {
		if (conf.video_palette_mode == 2) {
			fprintf(stderr, "Custom palette: not found: %s\n", filename);
			conf.video_palette_mode = 0;
		}
		return;
	}
	
	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	if (custompalette) { free(custompalette); }
	custompalette = malloc(filesize * sizeof(uint8_t));
	custpalsize = filesize * sizeof(uint8_t);
	
	result = fread(custompalette, sizeof(uint8_t), filesize, file);
	
	fclose(file);
}

void nst_palette_save() {
	// Save a custom palette
	FILE *file;
	void *custpalout;
	
	file = fopen(nstpaths.palettepath, "wb");
	if (!file) { return; }
	
	custpalout = malloc(custpalsize);
	
	memcpy(custpalout, custompalette, custpalsize);
	
	fwrite(custpalout, custpalsize, sizeof(uint8_t), file);
	fclose(file);
	free(custpalout);
}

void nst_palette_unload() {
	if (custompalette) { free(custompalette); }
}

bool nst_find_patch(char *patchname, unsigned int patchname_length, const char *filename) {
	// Check for a patch in the same directory as the game
	FILE *file;
	char filedir[512];

	// Copy filename (will be used by dirname)
	// dirname needs a copy because it can modify its argument
	strncpy(filedir, filename, sizeof(filedir));
	filedir[sizeof(filedir) - 1] = '\0';
	// Use memmove because dirname can return the same pointer as its argument,
	// since copying into same string as the argument we don't want any overlap
	memmove(filedir, dirname(filedir), sizeof(filedir));
	filedir[sizeof(filedir) - 1] = '\0';
	
	if (!conf.misc_soft_patching) { return 0; }
	
	snprintf(patchname, patchname_length, "%s/%s.ips", filedir, nstpaths.gamename);
	
	if ((file = fopen(patchname, "rb")) != NULL) { fclose(file); return 1; }
	else {
		snprintf(patchname, patchname_length, "%s/%s.ups", filedir, nstpaths.gamename);
		if ((file = fopen(patchname, "rb")) != NULL) { fclose(file); return 1; }
	}
	
	return 0;
}

void nst_set_callbacks() {
	// Set up the callbacks
	void *userData = (void*)0xDEADC0DE;
	
	Video::Output::lockCallback.Set(nst_cb_videolock, userData);
	Video::Output::unlockCallback.Set(nst_cb_videounlock, userData);
	
	Sound::Output::lockCallback.Set(nst_cb_soundlock, userData);
	Sound::Output::unlockCallback.Set(nst_cb_soundunlock, userData);
	
	User::fileIoCallback.Set(nst_cb_file, userData);
	User::logCallback.Set(nst_cb_log, userData);
	User::eventCallback.Set(nst_cb_event, userData);
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
	
	// Construct the custom palette path
	snprintf(nstpaths.palettepath, sizeof(nstpaths.palettepath), "%s%s", nstpaths.nstdir, "custom.pal");
	
	// Construct samples directory if it doesn't exist
	snprintf(dirstr, sizeof(dirstr), "%ssamples", nstpaths.nstdir);
#ifdef _MINGW	
	if (mkdir(dirstr) && errno != EEXIST) {
#else
	if (mkdir(dirstr, 0755) && errno != EEXIST) {
#endif
		fprintf(stderr, "Failed to create %s: %d\n", dirstr, errno);
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
	
	// Set up the sample directory
	snprintf(nstpaths.sampdir, sizeof(nstpaths.sampdir), "%ssamples/", nstpaths.nstdir);
	
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
void nst_set_overclock() {
	// Set video overclocking
	Video video(emulator);
	video.EnableOverclocking(conf.misc_overclock);
}

void nst_set_region() {
	// Set the region
	Machine machine(emulator);
	Cartridge::Database database(emulator);
	
	/*if (database.IsLoaded()) {
		std::ifstream dbfile(filename, std::ios::in|std::ios::binary);
		Cartridge::Profile profile;
		Cartridge::ReadInes(dbfile, nst_default_system(), profile);
		dbentry = database.FindEntry(profile.hash, nst_default_system());
		printf("Mapper: %d\n", dbentry.GetMapper());
	}*/
	
	switch (conf.misc_default_system) {
		case 0: machine.SetMode(machine.GetDesiredMode()); break; // Auto
		case 1: machine.SetMode(Machine::NTSC); break; // NTSC
		case 2: machine.SetMode(Machine::PAL); break; // PAL
		case 3: machine.SetMode(Machine::NTSC); break; // Famicom
		case 4: machine.SetMode(Machine::PAL); break; // Dendy
	}
}

void nst_set_rewind(int direction) {
	// Set the rewinder backward or forward
	switch (direction) {
		case 0: Rewinder(emulator).SetDirection(Rewinder::BACKWARD); break;
		case 1: Rewinder(emulator).SetDirection(Rewinder::FORWARD); break;
		default: break;
	}
}

void nst_state_save(char *filename) {
	// Save a state by filename
	Machine machine(emulator);
	
	std::ofstream statefile(filename, std::ifstream::out|std::ifstream::binary);
	
	if (statefile.is_open()) { machine.SaveState(statefile, Nes::Api::Machine::NO_COMPRESSION); }
	fprintf(stderr, "State Saved: %s\n", filename);
	nst_video_print("State Saved", 8, 212, 2, true);
}

void nst_state_load(char *filename) {
	// Load a state by filename
	Machine machine(emulator);
	
	std::ifstream statefile(filename, std::ifstream::in|std::ifstream::binary);
	
	if (statefile.is_open()) { machine.LoadState(statefile); }
	fprintf(stderr, "State Loaded: %s\n", filename);
	nst_video_print("State Loaded", 8, 212, 2, true);
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
		fprintf(stderr, "No State to Load\n");
		nst_video_print("No State to Load", 8, 212, 2, true);
		return;
	}
	
	nst_state_load(slotpath);
}

int nst_timing_runframes() {
	// Calculate how many emulation frames to run
	if (ffspeed) { return conf.timing_ffspeed; }
	return 1;
}

void nst_timing_set_ffspeed() {
	// Set the framerate to the fast-forward speed
	ffspeed = true;
}

void nst_timing_set_default() {
	// Set the framerate to the default
	ffspeed = false;
}

void nst_reset(bool hardreset) {
	// Reset the machine (soft or hard)
	Machine machine(emulator);
	Fds fds(emulator);
	machine.SetRamPowerState(conf.misc_power_state);
	machine.Reset(hardreset);
	
	// Set the FDS disk to defaults
	fds.EjectDisk();
	fds.InsertDisk(0, 0);
}

void nst_emuloop() {
	// Main Emulation Loop
	if (NES_SUCCEEDED(Rewinder(emulator).Enable(true))) {
		Rewinder(emulator).EnableSound(true);
	}
	
	if (playing) {
		audio_play();
		
		// Pulse the turbo buttons
		nst_input_turbo_pulse(cNstPads);
		
		// Execute frames
		for (int i = 0; i < nst_timing_runframes(); i++) {
			emulator.Execute(cNstVideo, cNstSound, cNstPads);
		}
	}
}

void nst_unload() {
	// Remove the cartridge and shut down the NES
	Machine machine(emulator);
	
	// Power down the NES
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
	//nstsdl_video_set_cursor();
}

void nst_play() {
	// Play the game
	if (playing) { return; }
	
	video_init();
	audio_init();
	nst_input_init();
	nst_cheats_init(nstpaths.cheatpath);
	nst_homebrew_init();

	cNstVideo = new Video::Output;
	cNstSound = new Sound::Output;
	cNstPads  = new Input::Controllers;
	
	audio_set_params(cNstSound);
	audio_unpause();
	
	if (nst_nsf()) {
		Nsf nsf(emulator);
		nsf.PlaySong();
		video_disp_nsf();
	}
	
	playing = true;
}

int nst_load(const char *filename) {
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
	static int loaded = 0;
	if (loaded) { nst_unload(); }
	nst_video_print_time("", false);
	
	// Check if the file is an archive and select the file within
	char reqfile[256]; // Requested file inside the archive
	if (nst_archive_select(filename, reqfile, sizeof(reqfile))) {
		// Extract the contents
		nst_archive_open(filename, &rom, &romsize, reqfile);
		
		// Convert the malloc'd char* to an istream
		std::string rombuf(rom, romsize);
		std::istringstream file(rombuf);
		free(rom);
		
		result = machine.Load(file, nst_default_system());
	}
	else { // Otherwise just load the file
		std::ifstream file(filename, std::ios::in|std::ios::binary);
		
		// Set the file paths
		nst_set_paths(filename);
		
		if (nst_find_patch(patchname, sizeof(patchname), filename)) { // Load with a patch if there is one
			std::ifstream pfile(patchname, std::ios::in|std::ios::binary);
			Machine::Patch patch(pfile, false);
			result = machine.Load(file, nst_default_system(), patch);
		}
		else { result = machine.Load(file, nst_default_system()); }
	}
	
	if (NES_FAILED(result)) {
		char errorstring[32];
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
		
		return 0;
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
	if (nst_nsf()) { nsf.StopSong(); }
	
	// Check if sound distortion should be enabled
	sound.SetGenie(conf.misc_genie_distortion);
	
	// Load the custom palette
	nst_palette_load(nstpaths.palettepath);
	
	// Set video overclocking
	nst_set_overclock();
	
	// Set the RAM's power state
	machine.SetRamPowerState(conf.misc_power_state);
	
	// Power on
	machine.Power(true);
	
	loaded = 1;
	return loaded;
}
