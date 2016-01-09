#ifndef _MAIN_H_
#define _MAIN_H_

#define VERSION "1.47"

typedef struct {
	char nstdir[256];
	char savedir[256];
	char gamename[256];
	char savename[512];
	char fdssave[512];
	char statepath[512];
	char cheatpath[512];
} nstpaths_t;

bool nst_archive_checkext(const char *filename);
bool nst_archive_handle(const char *filename, char **rom, int *romsize, const char *reqfile);
bool nst_find_patch(char *filename);
void nst_load_db();
void nst_load_fds_bios();
void nst_load(const char *filename);
void nst_play();
void nst_pause();
void nst_reset(bool hardreset);
void nst_schedule_quit();
void nst_set_dirs();
void nst_set_region();
void nst_set_rewind(int direction);

void nst_set_paths(const char *filename);

void nst_state_save(char *filename);
void nst_state_load(char *filename);
void nst_state_quicksave(int isvst);
void nst_state_quickload(int isvst);

void nst_movie_save(char *filename);
void nst_movie_load(char *filename);
void nst_movie_stop();

void nst_fds_info();
void nst_flip_disk();
void nst_switch_disk();

void nst_dipswitch();

#endif
