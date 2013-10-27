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
void NstLoadGame(const char* filename);
void NstLaunchConfig();

void SetupVideo();
void SetupSound();
void SetupInput();

void get_screen_res();
void ToggleFullscreen();
void FlipFDSDisk();
void SwitchFDSDisk();
void print_fds_info();
void print_message(char* message);
void nst_set_framerate();
void nst_set_dirs();

void QuickLoad(int isvst);
void QuickSave(int isvst);
void NstScheduleQuit();

void set_rewinder_direction(int direction);

#endif
