#ifndef _MAIN_H_
#define _MAIN_H_

void NstPlayGame();
void NstPlayNsf();
void NstStopNsf();
void NstScheduleQuit();
void NstStopPlaying();
bool NstIsPlaying();
void NstLoadGame(const char* filename);
void NstLaunchConfig();

#endif
