/*
 * Nestopia UE
 * 
 * Copyright (C) 2012-2018 R. Danbrook
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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#ifndef _MINGW
#include <archive.h>
#include <archive_entry.h>
#endif

#include "nstcommon.h"
#include "samples.h"

static uint8_t* wavfile;

extern nstpaths_t nstpaths;

int nst_sample_load_file(const char* filepath) {
	// Load a sample file into memory
	FILE *file;
	long filesize; // File size in bytes
	size_t result;
	
	file = fopen(filepath, "rb");
	
	if (!file) { return 0; }
	
	fseek(file, 0, SEEK_END);
	filesize = ftell(file);
	fseek(file, 0, SEEK_SET);
	
	wavfile = (uint8_t*)malloc(filesize * sizeof(uint8_t));
	
	if (wavfile == NULL) { return 0; }
	
	result = fread(wavfile, sizeof(uint8_t), filesize, file);
	if (result != filesize) { return 0; }
	
	fclose(file);
	return 1;
}

int nst_sample_load_archive(const char* filename, const char* reqfile) {
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
		return 0;
	}
	
	// Scan through the archive for files
	while (archive_read_next_header(a, &entry) == ARCHIVE_OK) {
		const char *currentfile = archive_entry_pathname(entry);
		int len = strlen(currentfile);
		if ((!strcasecmp(&currentfile[len-4], ".wav"))) {
			// Load the specified file
			if (!strcmp(currentfile, reqfile)) {
				entrysize = archive_entry_size(entry);
				wavfile = (uint8_t*)malloc(entrysize);
				archive_read_data(a, wavfile, entrysize);
				archive_read_data_skip(a);
				r = archive_read_free(a);
				return 1;
			}
		}
	}
#endif
	return 0;
}

int nst_sample_unload_file() {
	// Free the file
	if (wavfile) { free(wavfile); }
	return 1;
}

void nst_sample_setcontent(User::File& file) {
	// Parse the .WAV header and load the sample into the emulator
	
	// Check to see if it has a valid header
	uint8_t fmt[4] = { 0x66, 0x6d, 0x74, 0x20};
	uint8_t subchunk2id[4] = { 0x64, 0x61, 0x74, 0x61};
	if (memcmp(&wavfile[0x00], "RIFF", 4) != 0) { return; }
	if (memcmp(&wavfile[0x08], "WAVE", 4) != 0) { return; }
	if (memcmp(&wavfile[0x0c], &fmt, 4) != 0) { return; }
	if (memcmp(&wavfile[0x24], &subchunk2id, 4) != 0) { return; }
	
	// Load the sample into the emulator
	uint8_t *dataptr = &wavfile[0x2c];
	uint32_t datasize = wavfile[0x2b] << 24 | wavfile[0x2a] << 16 | wavfile[0x29] << 8 | wavfile[0x28];
	uint16_t blockalign = wavfile[0x21] << 8 | wavfile[0x20];
	uint16_t numchannels = wavfile[0x17] << 8 | wavfile[0x16];
	uint16_t bitspersample = wavfile[0x23] << 8 | wavfile[0x22];
	uint32_t samplerate = wavfile[0x1b] << 24 | wavfile[0x1a] << 16 | wavfile[0x19] << 8 | wavfile[0x18];
	file.SetSampleContent(dataptr, datasize / blockalign, numchannels == 2, bitspersample, samplerate);
}

void nst_sample_load_samples(User::File& file, const char* sampgame) {
	// Load samples for the specific game
	char reqfile[16];
	char samppath[576];
	
	// Requested sample .wav file
	snprintf(reqfile, sizeof(reqfile), "%02d.wav", file.GetId());
	
	// Check if there's a MAME-style zip archive
	snprintf(samppath, sizeof(samppath), "%s%s.zip", nstpaths.sampdir, sampgame);
	if (nst_sample_load_archive(samppath, reqfile)) {
		nst_sample_setcontent(file);
		nst_sample_unload_file();
	}
	else { // Otherwise load .wav files from an extracted directory
		snprintf(samppath, sizeof(samppath), "%s%s/%s", nstpaths.sampdir, sampgame, reqfile);
		if (nst_sample_load_file(samppath)) {
			nst_sample_setcontent(file);
			nst_sample_unload_file();
		}
	}
}
