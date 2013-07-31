#ifndef _MAIN_H_
#define _MAIN_H_

#define TV_WIDTH 292
#define OVERSCAN_LEFT 0
#define OVERSCAN_RIGHT 0
#define OVERSCAN_BOTTOM 8
#define OVERSCAN_TOP 8

void NstPlayGame();
void NstPlayNsf();
void NstStopNsf();
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

#endif
