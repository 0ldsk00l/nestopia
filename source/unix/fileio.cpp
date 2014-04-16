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

extern Nes::Api::Emulator emulator;
extern nstpaths_t nstpaths;

static std::ifstream *moviePlayFile, *fdsBiosFile, *nstDBFile;
static std::fstream *movieRecFile;

struct archive *a;
struct archive_entry *entry;
int r;

void fileio_init(void)
{
	moviePlayFile = NULL;
	movieRecFile = NULL;
	fdsBiosFile = NULL;
	nstDBFile = NULL;
}

void fileio_set_fds_bios(void) {
	
	Nes::Api::Fds fds(emulator);
	char biospath[512];
	
	if (fdsBiosFile) { return; }

	snprintf(biospath, sizeof(biospath), "%sdisksys.rom", nstpaths.nstdir);

	fdsBiosFile = new std::ifstream(biospath, std::ifstream::in|std::ifstream::binary);

	if (fdsBiosFile->is_open())
	{
		fds.SetBIOS(fdsBiosFile);
	}
	else
	{
		fprintf(stderr, "%s not found, Disk System games will not work.\n", biospath);
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

void fileio_load_db(void) {
	
	Nes::Api::Cartridge::Database database(emulator);
	char dbpath[512];

	if (nstDBFile) { return; }

	// Try to open the database file
	snprintf(dbpath, sizeof(dbpath), "%sNstDatabase.xml", nstpaths.nstdir);
	nstDBFile = new std::ifstream(dbpath, std::ifstream::in|std::ifstream::binary);
	
	if (nstDBFile->is_open()) {
		database.Load(*nstDBFile);
		database.Enable(true);
		return;
	}
#ifndef _MINGW
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
		fprintf(stderr, "NstDatabase.xml not found!\n");
		delete nstDBFile;
		nstDBFile = NULL;
	}
}

