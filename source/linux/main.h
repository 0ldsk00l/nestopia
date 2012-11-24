#ifndef _MAIN_H_
#define _MAIN_H_

void NstPlayGame();
void NstPlayNsf();
void NstStopNsf();
void NstScheduleQuit();
void NstStopPlaying();
bool NstIsPlaying();
bool NstIsLoaded();
void NstLoadGame(const char* filename);
void NstLaunchConfig();

void get_screen_res();

#endif
