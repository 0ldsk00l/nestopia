#ifndef _MAIN_H_
#define _MAIN_H_

#define VERSION "1.46-WIP"

void NstPlayGame();
void NstScheduleQuit();
void NstStopPlaying();
void NstSoftReset();
void NstHardReset();
bool NstIsPlaying();
bool NstIsLoaded();
void nst_load_game(const char *filename);

void SetupInput();

void FlipFDSDisk();
void SwitchFDSDisk();
void print_fds_info();
void print_message(char *message);
void nst_set_region();
void nst_set_dirs();

void QuickLoad(int isvst);
void QuickSave(int isvst);
void NstScheduleQuit();

void set_rewinder_direction(int direction);

#endif
