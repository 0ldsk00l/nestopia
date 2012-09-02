/***************************************************************************
   oss.c  -  M1 Linux audio output driver - supports SDL, OSS, and ALSA
 ***************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#include <SDL.h>

#define ALSA_PCM_NEW_HW_PARAMS_API
#define ALSA_PCM_NEW_SW_PARAMS_API
#include <alsa/asoundlib.h>

#include "oss.h"

#define VALGRIND 	(0)	// disables sound output for easier Valgrind usage

#define NUM_FRAGS_BROKEN    (8)
#define NUM_FRAGS_NORMAL    (4)
static INT32 num_frags;
#define OSS_FRAGMENT (0x000D | (num_frags<<16));  // 16k fragments (2 * 2^14).

// local variables
void  (*m1sdr_Callback)(unsigned long dwNumSamples, signed short *data);
unsigned long cbUserData;

static int hw_present, oss_pause;

static INT32 is_broken_driver;
int nDSoundSegLen = 0;
static int oss_nw = 0, oss_playing = 0;

int audiofd;
static snd_pcm_t *pHandle = NULL;
int lnxdrv_apimode = 0;		// 0 = SDL, 1 = ALSA, 2 = OSS

static INT16 samples[(48000*2)/10];

#define kMaxBuffers (4)		// this adjusts the latency for SDL audio

static volatile INT16 *buffer[kMaxBuffers];
static volatile int bufstat[kMaxBuffers];
static int playbuf, writebuf;
static Uint8 *curpos;
static int bytes_left;

static void fill_buffer(int bufnum)
{
	int bytes_to_fill, bufpos;
	Uint8 *bufptr;

	//	printf("FB%d\n", bufnum);

	// figure out how much we need out of this buffer
	// vs how much we can get out of it
	if (bytes_left >= bufstat[bufnum])
	{
		bytes_to_fill = bufstat[bufnum];
	}
	else
	{
		bytes_to_fill = bytes_left;
	}

	// copy from the buffer's current position
	bufptr = (Uint8 *)buffer[bufnum];
	bufpos = (nDSoundSegLen*2*sizeof(INT16)) - bufstat[bufnum];
	bufptr += bufpos;
	memcpy(curpos, bufptr, bytes_to_fill);

	// reduce the counters
	curpos += bytes_to_fill;
	bufstat[bufnum] -= bytes_to_fill;
	bytes_left -= bytes_to_fill;
}

static void sdl_callback(void *userdata, Uint8 *stream, int len)
{
	int temp;

	curpos = stream;
	bytes_left = len;

	// need more data?
	while (bytes_left > 0)
	{
		// does our current buffer have any samples?
		if (bufstat[playbuf] > 0)
		{
			fill_buffer(playbuf);
		}
		else
		{
			// check if the next buffer would collide
			temp = playbuf + 1;
			if (temp >= kMaxBuffers)
			{
				temp = 0;
			}

			// no collision, set it and continue looping
			if (temp != writebuf)
			{
				playbuf = temp;
			}
			else
			{
//			  printf("UF\n");
				// underflow!
				memset(curpos, 0, bytes_left);
				bytes_left = 0;
			}
		}
	}
}

// set # of samples per update
void m1sdr_SetSamplesPerTick(UINT32 spf)
{
	int i;

	nDSoundSegLen = spf;

	if (lnxdrv_apimode == 0) 
	{
		for (i = 0; i < kMaxBuffers; i++)
		{
			if (buffer[i])
			{
				free((void *)buffer[i]);
				buffer[i] = (volatile INT16 *)NULL;
			}

			buffer[i] = (volatile INT16 *)malloc(nDSoundSegLen * 2 * sizeof(UINT16));
			if (!buffer[i])
			{
				printf("Couldn't alloc buffer for SDL audio!\n");
				exit(-1);
			}

			memset((void *)buffer[i], 0, nDSoundSegLen * 2 * sizeof(UINT16));

			bufstat[i] = 0;
		}

		playbuf = 0;
		writebuf = 1;
	}
}

// m1sdr_Update - timer callback routine: runs sequencer and mixes sound
void m1sdr_Update(void)
{	
	if ((m1sdr_Callback) && (!oss_pause))
	{
		if (lnxdrv_apimode == 0) 
		{
	        	m1sdr_Callback(nDSoundSegLen, (INT16 *)buffer[writebuf]);
	
			bufstat[writebuf] = nDSoundSegLen * 2 * sizeof(UINT16);

			if (++writebuf >= kMaxBuffers)
			{
				writebuf = 0;
			}
		}
		else
		{
			m1sdr_Callback(nDSoundSegLen, (INT16 *)samples);
		}
	}

	if (oss_pause)
	{
		memset(samples, 0, nDSoundSegLen*4);
	}
}

// checks the play position to see if we should trigger another update
void m1sdr_TimeCheck(void)
{
	int timeout;
	snd_pcm_sframes_t delay = 0;

#if VALGRIND
	m1sdr_Update();
#else
	switch (lnxdrv_apimode)
	{
	case 0:	// SDL
		SDL_LockAudio();

		while ((bufstat[writebuf] == 0) && (writebuf != playbuf))
		{
			m1sdr_Update();
		}

		SDL_UnlockAudio();
		break;  

	case 1:	// ALSA
		if ((!pHandle) || (!oss_playing))
		{
			m1sdr_Update();
			return;
		}

		// get how many samples are buffered
		snd_pcm_delay(pHandle, &delay);

		// if we don't have at least 4 audio frames,
		// feed it more


		// HACK: ALSA likes to go completely batshit under some circumstances and claim either large
		// positive or large negative numbers are buffered.  These get things going again.
		if (delay < 0) delay = 0;
		if (delay > 1000000) delay = 10;

		timeout = 20;	// HACK 2: prevent ALSA from getting stuck when exiting a game
		while ((delay <= nDSoundSegLen*4) && (timeout > 0))
		{
			m1sdr_Update();
			snd_pcm_writei(pHandle, samples, nDSoundSegLen);
			snd_pcm_prepare(pHandle);

			snd_pcm_delay(pHandle, &delay);

			timeout--;
		}
		break;	

	case 2:	// OSS
		audio_buf_info info;

		if ((audiofd == -1) || (!oss_playing))
		{
			m1sdr_Update();
			return;
		}

	    	ioctl(audiofd, SNDCTL_DSP_GETOSPACE, &info);

		if (oss_nw)
		{
			int err;

			m1sdr_Update();

			// output the generated samples
			err = write(audiofd, samples, nDSoundSegLen * 4);
			if (err == -1)
			{
				perror("write\n");
			}
		}
		else
		{
		    	while (info.bytes >= (nDSoundSegLen * 4))
			{
				m1sdr_Update();

				// output the generated samples
				write(audiofd, samples, nDSoundSegLen * 4);

			    	ioctl(audiofd, SNDCTL_DSP_GETOSPACE, &info);
			}
		}
		break;
	}

	usleep(50);
#endif
}

// m1sdr_Init - inits the output device and our global state

INT16 m1sdr_Init(int sample_rate)
{	
	int format, stereo, rate, fsize, err;
	unsigned int nfreq, periodtime;
	snd_pcm_hw_params_t *hwparams;

	hw_present = 0;

	m1sdr_Callback = NULL;

	switch (lnxdrv_apimode)
	{
	case 0: // SDL
		SDL_AudioSpec aspec;

		SDL_InitSubSystem(SDL_INIT_AUDIO);

	 	m1sdr_SetSamplesPerTick(sample_rate/60);	

		playbuf = 0;
		writebuf = 1;

		aspec.freq = sample_rate;
		aspec.format = AUDIO_S16SYS;	// keep endian independant 
		aspec.channels = 2;
		aspec.samples = 512;		// has to be a power of 2, and we want it smaller than our buffer size
		aspec.callback = sdl_callback;
		aspec.userdata = 0;

		if (SDL_OpenAudio(&aspec, NULL) < 0)
		{
			printf("ERROR: can't open SDL audio\n");
			return 0;
		}

		// make sure we don't start yet
		SDL_PauseAudio(1);
		break;
				
	case 1:	// ALSA
		// Try to open audio device
		if ((err = snd_pcm_open(&pHandle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) < 0) {
			fprintf(stderr, "ALSA: Could not open soundcard (%s)\n", snd_strerror(err));
			hw_present = 0;
			return 0;
		}

		if ((err = snd_pcm_hw_params_malloc(&hwparams)) < 0) {
			fprintf (stderr, "cannot allocate hardware parameter structure (%s)\n",
				 snd_strerror(err));
			return 0;
		}

		// Init hwparams with full configuration space
		if ((err = snd_pcm_hw_params_any(pHandle, hwparams)) < 0) {
			fprintf(stderr, "ALSA: couldn't set hw params (%s)\n", snd_strerror(err));
			hw_present = 0;
			return 0;
		}

		// Set access type
		if ((err = snd_pcm_hw_params_set_access(pHandle, hwparams, SND_PCM_ACCESS_RW_INTERLEAVED)) < 0) {
			fprintf(stderr, "ALSA: can't set access (%s)\n", snd_strerror(err));
			return 0;
		}

		// Set sample format
		if ((err = snd_pcm_hw_params_set_format(pHandle, hwparams, SND_PCM_FORMAT_S16)) < 0) {
			fprintf(stderr, "ALSA: can't set format (%s)\n", snd_strerror(err));
			return 0;
		}

		// Set sample rate (nearest possible)
		nfreq = sample_rate;
		if ((err = snd_pcm_hw_params_set_rate_near(pHandle, hwparams, &nfreq, 0)) < 0) {
			fprintf(stderr, "ALSA: can't set sample rate (%s)\n", snd_strerror(err));
			return 0;
		}

		// Set number of channels
		if ((err = snd_pcm_hw_params_set_channels(pHandle, hwparams, 2)) < 0) {
			fprintf(stderr, "ALSA: can't set stereo (%s)\n", snd_strerror(err));
			return 0;
		}

		// Set period time (nearest possible)
		periodtime = 20000;
		if ((err = snd_pcm_hw_params_set_period_time_near(pHandle, hwparams, &periodtime, 0)) < 0) {
			fprintf(stderr, "ALSA: can't set period time (%s)\n", snd_strerror(err));
			return 0;
		}

		// Apply HW parameter settings to PCM device and prepare device
		if ((err = snd_pcm_hw_params(pHandle, hwparams)) < 0) {
			fprintf(stderr, "ALSA: unable to install hw_params (%s)\n", snd_strerror(err));
			snd_pcm_hw_params_free(hwparams);
			return 0;
		}

		snd_pcm_hw_params_free(hwparams);

		if ((err = snd_pcm_prepare(pHandle)) < 0) {
			fprintf (stderr, "cannot prepare audio interface for use (%s)\n", snd_strerror(err));
			return 0;
		}
		break;	

	case 2:	// OSS
		audiofd = open("/dev/dsp", O_WRONLY, 0);
		if (audiofd == -1)
		{
			perror("/dev/dsp");
			printf("ERROR: unable to open soundcard.  Aborting.\n");
			return(0);
		}

		// reset things
		ioctl(audiofd, SNDCTL_DSP_RESET, 0);

		is_broken_driver = 0;
		num_frags = NUM_FRAGS_NORMAL;

		// set the buffer size we want
		fsize = OSS_FRAGMENT;
		if (ioctl(audiofd, SNDCTL_DSP_SETFRAGMENT, &fsize) == - 1)
		{
			perror("SNDCTL_DSP_SETFRAGMENT");
			return(0);
		}

		// set 16-bit output
		format = AFMT_S16_NE;	// 16 bit signed "native"-endian
		if (ioctl(audiofd, SNDCTL_DSP_SETFMT, &format) == - 1)
		{
			perror("SNDCTL_DSP_SETFMT");
			return(0);
		}

		// now set stereo
		stereo = 1;
		if (ioctl(audiofd, SNDCTL_DSP_STEREO, &stereo) == - 1)
		{
			perror("SNDCTL_DSP_STEREO");
			return(0);
		}

		// and the sample rate
		rate = sample_rate;
		if (ioctl(audiofd, SNDCTL_DSP_SPEED, &rate) == - 1)
		{
			perror("SNDCTL_DSP_SPEED");
			return(0);
		}

		// and make sure that did what we wanted
		ioctl(audiofd, SNDCTL_DSP_GETBLKSIZE, &fsize);
		break;
	}

	hw_present = 1;

	return (1);
}

void m1sdr_Exit(void)
{	
	int i;

	if (!hw_present) return;

	switch (lnxdrv_apimode)
	{
	case 0:	// SDL
		SDL_QuitSubSystem(SDL_INIT_AUDIO);

		for (i = 0; i < kMaxBuffers; i++)
		{
			if (buffer[i])
			{
				free((void *)buffer[i]);
				buffer[i] = (volatile INT16 *)NULL;
			}
		}
		break;	

	case 1:	// ALSA
		snd_pcm_close(pHandle);
		break;	

	case 2:	// OSS
		close(audiofd);
		break;	
	}
}

void m1sdr_SetCallback(void *fn)
{
	if (fn == (void *)NULL)
	{
		printf("ERROR: NULL CALLBACK!\n");
	}

	m1sdr_Callback = (void (*)(unsigned long, signed short *))fn;
}

INT16 m1sdr_IsThere(void)
{
	int err;

	if (lnxdrv_apimode == 1)
	{
		if ((err = snd_pcm_open(&pHandle, "plughw:0,0", SND_PCM_STREAM_PLAYBACK, 0)) != 0)
		{
			printf("Error accessing soundcard, sound will be disabled\n");
			hw_present = 0;
			return(0);
		}

		snd_pcm_close(pHandle);
		hw_present = 1;
	}
	else if (lnxdrv_apimode == 2)
	{
		audiofd = open("/dev/dsp", O_WRONLY, 0);

		if (audiofd == -1)
		{
			printf("Error accessing soundcard, sound will be disabled\n");
			hw_present = 0;
			return(0);
		}

		close(audiofd);
		hw_present = 1;
	}
	else if (lnxdrv_apimode == 0)
	{
		hw_present = 1;	// always say it's present for SDL
	}

	return (1);
}

INT32 m1sdr_HwPresent(void)
{
	return hw_present;
}

void m1sdr_PlayStart(void)
{
	if (lnxdrv_apimode == 0) 
	{
		SDL_PauseAudio(0);
	}
	oss_playing = 1;
}

void m1sdr_PlayStop(void)
{
	if (lnxdrv_apimode == 0) 
	{
		SDL_PauseAudio(1);
	}
	oss_playing = 0;
}

void m1sdr_FlushAudio(void)
{
	memset(samples, 0, nDSoundSegLen * 4);
	if (lnxdrv_apimode == 2) 
	{
		write(audiofd, samples, nDSoundSegLen * 4);
		write(audiofd, samples, nDSoundSegLen * 4);
	}
}

void m1sdr_Pause(int set)
{
	oss_pause = set;
}

void m1sdr_SetNoWait(int nw)
{
	oss_nw = nw;
}
