#ifndef _NSTCOMMON_H_
#define _NSTCOMMON_H_

// Nst Core
#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiSound.hpp"
#include "core/api/NstApiMachine.hpp"
#include "core/api/NstApiCartridge.hpp"
#include "core/api/NstApiFds.hpp"
#include "core/api/NstApiNsf.hpp"
#include "core/api/NstApiUser.hpp"
#include "core/api/NstApiRewinder.hpp"
#include "core/api/NstApiMovie.hpp"

using namespace Nes::Api;

typedef struct {
	char nstdir[256];
	char savedir[256];
	char gamename[256];
	char savename[512];
	char fdssave[512];
	char statepath[512];
	char cheatpath[512];
	char palettepath[512];
	char sampdir[512];
} nstpaths_t;

// Pointers
void* nst_ptr_video();
void* nst_ptr_sound();
void* nst_ptr_input();

// Archives
bool nst_archive_checkext(const char *filename);
bool nst_archive_select_file(const char *filename, char *reqfile, size_t reqsize);
bool nst_archive_open(const char *filename, char **rom, int *romsize, const char *reqfile);

// DIP Switches
void nst_dipswitch();

// Database
void nst_db_load();
void nst_db_unload();

// FDS
void nst_fds_bios_load();
void nst_fds_bios_unload();
void nst_fds_info();
void nst_fds_flip();
void nst_fds_switch();

// Movies
void nst_movie_save(char *filename);
void nst_movie_load(char *filename);
void nst_movie_stop();

// NSF
bool nst_nsf();
void nst_nsf_play();
void nst_nsf_stop();
void nst_nsf_prev();
void nst_nsf_next();

// PAL
bool nst_pal();

// Play check
bool nst_playing();

// Palette
void nst_palette_load(const char *filename);
void nst_palette_save();
void nst_palette_unload();

// Patch
bool nst_find_patch(char *patchname, unsigned int patchname_length, const char *filename);

// Setters
void nst_set_callbacks();
void nst_set_dirs();
void nst_set_overclock();
void nst_set_paths(const char *filename);
void nst_set_region();
void nst_set_rewind(int direction);

// States
void nst_state_save(char *filename);
void nst_state_load(char *filename);
void nst_state_quicksave(int isvst);
void nst_state_quickload(int isvst);

// Timing
int nst_timing_runframes();
void nst_timing_set_ffspeed();
void nst_timing_set_default();

// Main Emulation
void nst_reset(bool hardreset);
void nst_emuloop();

// Unsorted
int nst_load(const char *filename);
void nst_unload();
void nst_play();
void nst_pause();

#endif
