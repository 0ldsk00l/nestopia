#ifndef _OSS_H_
#define _OSS_H_

typedef signed   char      INT8;
typedef signed   short     INT16;
typedef signed   int       INT32;
typedef signed   long long INT64;
typedef unsigned char      UINT8;
typedef unsigned short     UINT16;
typedef unsigned int       UINT32;
typedef unsigned long long UINT64;

extern int audiofd;
extern void (*m1sdr_Callback)(unsigned long dwUser, signed short *smp);
extern unsigned long cbUserData;

// function protos

void m1sdr_Update(void);
INT16 m1sdr_Init(int sample_rate);
void m1sdr_Exit(void);
void m1sdr_PlayStart(void);
void m1sdr_PlayStop(void);
INT16 m1sdr_IsThere(void);
void m1sdr_TimeCheck(void);
void m1sdr_SetSamplesPerTick(UINT32 spf);
void m1sdr_SetHz(UINT32 hz);
void m1sdr_SetCallback(void *fn);
INT32 m1sdr_HwPresent(void);
void m1sdr_FlushAudio(void);
void m1sdr_Pause(int); 
void m1sdr_SetNoWait(int nw);
#endif
