/*
	NEStopia / Linux
	Port by R. Belmont
	
	main.cpp - main file
*/

#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <iomanip>
#include <SDL.h>
#include <string.h>
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <vector>

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiSound.hpp"
#include "core/api/NstApiInput.hpp"
#include "core/api/NstApiMachine.hpp"
#include "core/api/NstApiUser.hpp"
#include "core/api/NstApiNsf.hpp"
#include "core/api/NstApiMovie.hpp"
#include "core/api/NstApiFds.hpp"
#include "core/api/NstApiRewinder.hpp"
#include "core/api/NstApiCartridge.hpp"
#include "core/api/NstApiCheats.hpp"
#include "core/NstCrc32.hpp"
#include "core/NstChecksum.hpp"
#include "core/NstXml.hpp"
#include "oss.h"
#include "settings.h"
#include "auxio.h"
#include "input.h"
#include "controlconfig.h"
#include "cheats.h"
#include "seffect.h"
#include "main.h"
#include "GL/glu.h"

#define NST_VERSION "1.4.0 [release H]"

extern "C" {
#include <gtk/gtk.h>

#include "callbacks.h"
}

#include "uihelp.h"

using namespace Nes::Api;
using namespace LinuxNst;

// base class, all interfaces derives from this
Emulator emulator;

// forward declaration
void SetupVideo();
void SetupSound();
void SetupInput();

SDL_Surface *screen;

static short lbuf[48000];
static long exholding[48000*2];

static unsigned short keys[65536];

static int updateok, playing = 0, cur_width, cur_Rwidth, cur_height, cur_Rheight, loaded = 0, framerate;
static int nst_quit = 0, nsf_mode = 0, state_save = 0, state_load = 0, movie_save = 0, movie_load = 0, movie_stop = 0;
static int schedule_stop = 0;
static SDL_Joystick *joy[10];

extern int lnxdrv_apimode;
extern GtkWidget *mainwindow;

static char savename[512], capname[512], gamebasename[512];
static char caption[128];
char rootname[512], lastarchname[512];

static InputDefT *ctl_defs;

static Video::Output *cNstVideo;
static Sound::Output *cNstSound;
static Input::Controllers *cNstPads;
static Cartridge::Database::Entry dbentry;

static Settings *sSettings;
static CheatMgr *sCheatMgr;

static bool         using_opengl  = false;
static bool         linear_filter  = false;
static GLuint       screenTexID  = 0;
static int          gl_w, gl_h;

static void         *intbuffer;	// intermediate buffer: the NST engine draws into this, and we may blit it
				// either as-is or in other ways

// get the favored system selected by the user
static Machine::FavoredSystem get_favored_system(void)
{
	switch (sSettings->GetPrefSystem())
	{
		case 0:
			return Machine::FAVORED_NES_NTSC;
			break;

		case 1:
			return Machine::FAVORED_NES_PAL;
			break;

		case 2:
			return Machine::FAVORED_FAMICOM;
			break;

		case 3:
			return Machine::FAVORED_DENDY;
			break;
	}

	return Machine::FAVORED_NES_NTSC;
}

// convert a number into the next highest power of 2
static int powerOfTwo( const int value )
{
	int result = 1;
	while ( result < value )
		result <<= 1;
	return result;	
}

// init OpenGL and set up for blitting
static void opengl_init_structures()
{
	int scalefactor =  sSettings->GetScaleAmt() + 1;

	glEnable( GL_TEXTURE_2D );

	gl_w = powerOfTwo(cur_width);
	gl_h = powerOfTwo(cur_height);

	glGenTextures( 1, &screenTexID ) ;
	glBindTexture( GL_TEXTURE_2D, screenTexID ) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, linear_filter ? GL_LINEAR : GL_NEAREST) ;
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST ) ;

	glViewport( 0,0, screen->w, screen->h );
	glDisable( GL_DEPTH_TEST );
	glDisable( GL_ALPHA_TEST );
	glDisable( GL_BLEND );
	glDisable( GL_LIGHTING );
	glDisable( GL_TEXTURE_3D_EXT );
	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();
	glOrtho(0.0, (GLdouble)screen->w, (GLdouble)screen->h, 0.0, 0.0, -1.0);
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

// tears down OpenGL when it's no longer needed
static void opengl_cleanup()
{
	if (using_opengl)
	{
		SDL_FreeSurface( screen );
		glDeleteTextures( 1, &screenTexID );
		if (intbuffer)
		{
			free(intbuffer);
			intbuffer = NULL;
		}
	}
}

// blit the image using OpenGL
static void opengl_blit()
{
	double gl_blit_width = (double)cur_width / (double)gl_w;
	double gl_blit_height = (double)cur_height / (double)gl_h;

	glTexImage2D( GL_TEXTURE_2D,
    	          0,
    	          GL_RGBA,
    	          gl_w, gl_h,
    	          0,
    	          GL_BGRA,
    	          GL_UNSIGNED_BYTE,
		  intbuffer  ) ;
	glBegin( GL_QUADS ) ;
		glTexCoord2f(gl_blit_width, gl_blit_height); glVertex2i(cur_Rwidth, cur_Rheight);
		glTexCoord2f(gl_blit_width, 0.0f ); glVertex2i(cur_Rwidth,  0);
		glTexCoord2f(0.0f, 0.0f ); glVertex2i(0, 0);
		glTexCoord2f(0.0f, gl_blit_height); glVertex2i(0, cur_Rheight);
	glEnd();

	SDL_GL_SwapBuffers();	
}

// *******************
// emulation callbacks
// *******************

long Linux_LockScreen(void*& ptr)
{
	if (using_opengl)		// have the engine blit directly to our memory buffer
	{
		ptr = intbuffer;
		return gl_w*4;
	}
	else
	{
		if (SDL_MUSTLOCK(screen)) SDL_LockSurface(screen);

		ptr = intbuffer;
	}
	return screen->pitch;
}

void Linux_UnlockScreen(void*)
{
	if (using_opengl)
	{
		opengl_blit();
	}
	else
	{
		unsigned short *src, *dst1;
		unsigned int *srcL, *dst1L;
		int x, y, vdouble;

		// is this a software x2 expand for NTSC mode?
		vdouble = 0;
		if (screen->h == (cur_height<<1))
		{
			vdouble = 1;
		}

		if (screen->format->BitsPerPixel == 16)
		{
			src = (UINT16 *)intbuffer;
			dst1 = (UINT16 *)screen->pixels;

			for (y = 0; y < cur_Rheight; y++)
			{
				memcpy(dst1, src, cur_width*screen->format->BitsPerPixel/8);
				if (vdouble)
				{
					if (!(y & 1))
					{
						src += cur_width;
					}
				}
				else
				{
					src += cur_width;
				}
				dst1 += screen->pitch/2;
			}
		}
		else if (screen->format->BitsPerPixel == 32)
		{
			srcL = (UINT32 *)intbuffer;
			dst1L = (UINT32 *)screen->pixels;

			for (y = 0; y < cur_Rheight; y++)
			{
				memcpy(dst1L, srcL, cur_width*screen->format->BitsPerPixel/8);
				if (vdouble)
				{
					if (!(y & 1))
					{
						srcL += cur_width;
					}
				}
				else
				{
					srcL += cur_width;
				}
				dst1L += screen->pitch/4;
			}
		}
		else printf("ERROR: Unknown pixel format %d bpp\n", screen->format->BitsPerPixel);

		if (SDL_MUSTLOCK(screen)) SDL_UnlockSurface(screen);
		SDL_Flip(screen);
	}

}

// called right before Nestopia is about to write pixels
static bool NST_CALLBACK VideoLock(void* userData, Video::Output& video)
{
	if (nsf_mode) return false;

	video.pitch = Linux_LockScreen( video.pixels );
	return true; // true=lock success, false=lock failed (Nestopia will carry on but skip video)
}

// called right after Nestopia has finished writing pixels (not called if previous lock failed)
static void NST_CALLBACK VideoUnlock(void* userData, Video::Output& video)
{
	if (nsf_mode) return;

	Linux_UnlockScreen( video.pixels );
}

// callback to feed a frame of audio to the output driver
void nst_do_frame(unsigned long dwSamples, signed short *out)
{
	int s;
	short *pbufL = (short *)lbuf;
	short *outbuf;
	long dtl, dtr;

	outbuf = out;
	if (nsf_mode)
	{
		Nsf nsf( emulator );

		if (!nsf.IsPlaying())
		{
			for (s = 0; s < dwSamples; s++)
			{
				*out++ = 0;
				*out++ = 0;
			}
			return;
		}
	}

	if (sSettings->GetUseExciter())
	{
		int j = 0;

		if (!sSettings->GetStereo())
		{
			// exciter can't handle "hot" samples, so
			// tone them down a bit
			for (s = 0; s < dwSamples; s++)
			{
				exholding[j++] = (*pbufL)/4;
				exholding[j++] = (*pbufL++)/4;
			}
		}
		else	// stereo
		{
			for (s = 0; s < dwSamples; s++)
			{
				exholding[j++] = (*pbufL++)/4;
				exholding[j++] = (*pbufL++)/4;
			}

			seffect_ex_process((long *)exholding, dwSamples);

			j = 0;
			for (s = 0; s < dwSamples; s++)
			{
				dtr = exholding[j++];
				dtl = exholding[j++];

				if(dtl>0x7fffL)	dtl=0x7fffL;
				else if(dtl<-0x7fffL) dtl=-0x7fffL;
				if(dtr>0x7fffL)	dtr=0x7fffL;
				else if(dtr<-0x7fffL) dtr=-0x7fffL;
				
				*out++ = (dtr & 0xffff);
				*out++ = (dtl & 0xffff);
			}
		}

	}
	else
	{
		if (!sSettings->GetStereo())
		{
			for (s = 0; s < dwSamples; s++)
			{
				*out++ = *pbufL;
				*out++ = *pbufL++;
			}
		}
		else	// stereo
		{
			for (s = 0; s < dwSamples; s++)
			{
				*out++ = *pbufL++;
				*out++ = *pbufL++;
			}
		}
	}

	if (sSettings->GetUseSurround())
	{
		seffect_surround_lite_process(outbuf, dwSamples*4);
	}
	updateok = 1;
}

// do a "partial" shutdown
static void nst_unload2(void)
{
	Machine machine(emulator);

	// if nothing's loaded, do nothing
	if (!loaded)
	{
		return;
	}

	// power down the emulated NES
	std::cout << "Powering down the emulated machine\n";
	machine.Power(false);

	// unload the cart
	machine.Unload();

	// erase any cheats
	sCheatMgr->Unload();
}

static void nst_unload(void)
{
	nst_unload2();

	UIHelp_Unload();
}

// if we're in full screen, kills video temporarily
static void kill_video_if_fs(void)
{
	if (sSettings->GetFullscreen())
	{
		if (SDL_NumJoysticks() > 0)
		{
			int i;

			for (i = 0; i < SDL_NumJoysticks(); i++)
			{
				// we only? support 10 joysticks
				if (i < 10)
				{
					SDL_JoystickClose(joy[i]);
				}
			}

			SDL_JoystickEventState(SDL_ENABLE);	// turn on regular updates
		}

		SDL_ShowCursor(1);
		SDL_Quit();
	}
}

// returns if we're currently playing a game or NSF
bool NstIsPlaying()
{
	return playing;
}

// shuts everything down
void NstStopPlaying()
{
	if (playing)
	{
		int i;

		// kill any movie
		auxio_do_movie_stop();

		// close video sanely
		if (!nsf_mode)
		{
			SDL_FreeSurface(screen);
			opengl_cleanup();
			if (intbuffer)
			{
				free(intbuffer);
				intbuffer = NULL;
			}
		}

		// get machine interface...
		Machine machine(emulator);

		// shut down the sound system too
		m1sdr_PlayStop();
		m1sdr_Exit();

		// flush the sound buffer
		memset(lbuf, 0, sizeof(lbuf));

		// kill SDL
		if (SDL_NumJoysticks() > 0)
		{
			for (i = 0; i < SDL_NumJoysticks(); i++)
			{
				// we only? support 10 joysticks
				if (i < 10)
				{
					SDL_JoystickClose(joy[i]);
				}
			}

			SDL_JoystickEventState(SDL_ENABLE);	// turn on regular updates
		}
		SDL_ShowCursor(1);
		SDL_Quit();
	}

	// show main window
	gtk_widget_show(mainwindow);

	playing = 0;
}

#define CRg(rg) (sizeof(rg) / sizeof(rg[0]))
std::string svst[2];

// generate the filename for quicksave files
std::string StrQuickSaveFile(int isvst)
{
	const char *home = getenv("HOME");
	if (!home)
		{
		std::cout << "couldn't get home directory\n";
		return "";
		}
	std::ostringstream ossFile;
	ossFile << home;
	ossFile << "/.nestopia/qsave";

	if (mkdir(ossFile.str().c_str(), 0777) && errno != EEXIST)
		{
		std::cout << "couldn't make qsave directory: " << errno << "\n";
		return "";
		}
      	
	ossFile << "/" << std::setbase(16) << std::setfill('0') << std::setw(8)
		<< basename(gamebasename) << std::string("_") << isvst << ".nst";

	return ossFile.str();
}

// save state to memory slot
static void QuickSave(int isvst)
{
	std::string strFile = StrQuickSaveFile(isvst);
	if (strFile.empty())
		return;

	Machine machine( emulator );
	std::ofstream os(strFile.c_str());
	// use "NO_COMPRESSION" to make it easier to hack save states
	Nes::Result res = machine.SaveState(os, Nes::Api::Machine::USE_COMPRESSION);
}


// restore state from memory slot
static void QuickLoad(int isvst)
{
	std::string strFile = StrQuickSaveFile(isvst);
	if (strFile.empty())
		return;

	Machine machine( emulator );
	std::ifstream is(strFile.c_str());
	Nes::Result res = machine.LoadState(is);
}


// start playing
void NstPlayGame(void)
{
	// hide main window
	gtk_widget_hide(mainwindow);

	// process pending gtk events
	while (gtk_events_pending())
	{
		gtk_main_iteration();
	}

	// initialization
	SetupVideo();
	SetupSound();
	SetupInput();

	// apply any cheats into the engine
	sCheatMgr->Enable();

	cNstVideo = new Video::Output;
	cNstSound = new Sound::Output;
	cNstPads  = new Input::Controllers;

	cNstSound->samples[0] = lbuf;
	cNstSound->length[0] = sSettings->GetRate()/framerate;
	cNstSound->samples[1] = NULL;
	cNstSound->length[1] = 0;

	SDL_WM_SetCaption(caption, caption);

	m1sdr_SetSamplesPerTick(cNstSound->length[0]);

	updateok = 0;
	schedule_stop = 0;
	playing = 1;
}

// start playing an NSF file
void NstPlayNsf(void)
{
	Nsf nsf( emulator );
	
	nsf.PlaySong();
}

// stop playing an NSF file
void NstStopNsf(void)
{
	Nsf nsf( emulator );
	
	nsf.StopSong();

	// clear the audio buffer
	memset(lbuf, 0, sizeof(lbuf));
}

// schedule a NEStopia quit
void NstScheduleQuit(void)
{
	nst_quit = 1;
}

// launch the controller configurator
void NstLaunchConfig(void)
{
	run_configurator(ctl_defs, sSettings->GetConfigItem(), sSettings->GetUseJoypads());
}

// toggle fullscreen state
static void ToggleFullscreen()
{
	if (SDL_NumJoysticks() > 0)
	{
		for (int i = 0; i < SDL_NumJoysticks(); i++)
		{
			// we only? support 10 joysticks
			if (i < 10)
			{
				SDL_JoystickClose(joy[i]);
			}
		}

		SDL_JoystickEventState(SDL_ENABLE);	// turn on regular updates
	}

	SDL_ShowCursor(1);
	SDL_FreeSurface(screen);
	opengl_cleanup();

	if (intbuffer)
	{
		free(intbuffer);
		intbuffer = NULL;
	}

	SDL_Quit();
	sSettings->SetFullscreen(sSettings->GetFullscreen()^1);
	SetupVideo();

	lnxdrv_apimode = sSettings->GetSndAPI();
	if (lnxdrv_apimode == 0) 	// the SDL driver needs a harder restart
	{
		m1sdr_Exit();
		m1sdr_Init(sSettings->GetRate());
		m1sdr_SetCallback((void *)nst_do_frame);
		m1sdr_PlayStart();
	}


	SDL_WM_SetCaption(caption, caption);
}


// handle input event
static void handle_input_event(Input::Controllers *controllers, InputEvtT inevt)
{
	#ifdef DEBUG_INPUT
	printf("metaevent: %d\n", (int)inevt);
	#endif
	switch (inevt)
	{
	case MSAVE:
		movie_save = 1;
		break;
	case MLOAD:
		movie_load = 1;
		break;
	case MSTOP:
		movie_stop = 1;
		break;

	case RESET:
		{
			Machine machine( emulator );
			Fds fds( emulator );

			machine.Reset(true);

			// put the disk system back to disk 0 side 0
			fds.EjectDisk();
			fds.InsertDisk(0, 0);
		}
		break;

	case FLIP:
		{
		Fds fds( emulator );

		if (fds.CanChangeDiskSide())
			fds.ChangeSide();
		}
		break;

	case FSCREEN:
		ToggleFullscreen();
		break;

	case RBACK:
		Rewinder(emulator).SetDirection(Rewinder::BACKWARD);
		break;
	case RFORE:
		Rewinder(emulator).SetDirection(Rewinder::FORWARD);
		break;

	case QSAVE1:
		QuickSave(0);
		break;
	case QLOAD1:
		QuickLoad(0);
		break;
	case QSAVE2:
		QuickSave(1);
		break;
	case QLOAD2:
		QuickLoad(1);
		break;

	case SAVE:
		state_save = 1;
		break;
	case LOAD:
		state_load = 1;
		break;

	case STOP:
		schedule_stop = 1;
		break;
	case EXIT:
		schedule_stop = 1;
		nst_quit = 1;
		break;
	case COIN1:
		controllers->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_1;
		break;
	case COIN2:
		controllers->vsSystem.insertCoin |= Input::Controllers::VsSystem::COIN_2;
		break;

	default:
		assert(0);
	}
}


// match input event; if pind is not NULL, continue after it
// on is set if the key/button is hit, clear if key is up/axis centered
static const InputDefT *nst_match(const SDL_Event &evt, const InputDefT *pind, bool &on)
{
	pind = (pind == NULL) ? ctl_defs : pind + 1;

	bool match = false;
	for (; pind->player != -1 && !match; ++pind)
	{
		switch (evt.type)
		{
			case SDL_KEYDOWN:
			case SDL_KEYUP:
				#ifdef DEBUG_INPUT
				if (evt.type == SDL_KEYDOWN)
				{
					printf("key is down: sym %x mod %x vs sym %x mod %x\n", evt.key.keysym.sym, evt.key.keysym.mod, pind->evt.key.keysym.sym, pind->evt.key.keysym.mod);
				}
				#endif

				match = (pind->evt.type == SDL_KEYDOWN && pind->evt.key.keysym.sym == evt.key.keysym.sym);
				// do better mod checking
				if ((pind->evt.key.keysym.mod & evt.key.keysym.mod) != pind->evt.key.keysym.mod)
				{
					match = 0;
				}

				on = (evt.key.state == SDL_PRESSED);
				break;

			case SDL_JOYBUTTONDOWN:
			case SDL_JOYBUTTONUP:
				match = pind->evt.type == SDL_JOYBUTTONDOWN
					&& pind->evt.jbutton.which == evt.jbutton.which
					&& pind->evt.jbutton.button == evt.jbutton.button;
				on = (evt.jbutton.state == SDL_PRESSED);
				break;

			case SDL_JOYAXISMOTION:
			{
				const Sint16 nvalue = (abs(evt.jaxis.value) < DEADZONE) ? 0 :
					(evt.jaxis.value < 0) ? -1 : 1; 	// normalized axis direction
				match = pind->evt.type == evt.type
					&& pind->evt.jaxis.which == evt.jaxis.which
					&& pind->evt.jaxis.axis == evt.jaxis.axis
					&& (nvalue == 0 || pind->evt.jaxis.value == nvalue);
				on = nvalue != 0;
				break;
			}
		}

		if (match)
		{
			return pind;
		}
	}

	return NULL;
}


// try to dispatch an input event
static void nst_dispatch(Input::Controllers *controllers, const SDL_Event &evt)
{
	bool on;
	const InputDefT *pind = NULL;

	controllers->vsSystem.insertCoin = 0;

	while ((pind = nst_match(evt, pind, on)) != NULL)
	{
		if (on)
		{
			if (pind->player == 0)
            {    
				handle_input_event(controllers, (InputEvtT)pind->codeout);
			}
			else
            {    
				#ifdef DEBUG_INPUT
				printf("player %d event: codeout %x\n", pind->player, pind->codeout);
				#endif
				controllers->pad[pind->player - 1].buttons |= pind->codeout;
			}
		}
		else
		{
			if (pind->player != 0)
            {    
				controllers->pad[pind->player - 1].buttons &= ~pind->codeout;
			}
		}
	}
}

// logging callback called by the core
static void NST_CALLBACK DoLog(void *userData, const char *string, ulong length)
{
	fprintf(stderr, "%s", string);
}

// for various file operations, usually called during image file load, power on/off and reset
static void NST_CALLBACK DoFileIO(void *userData, User::File& file)
{
	unsigned char *compbuffer;
	int compsize, compoffset;
	char mbstr[512];

	switch (file.GetAction())
	{
		case User::File::LOAD_ROM:
			wcstombs(mbstr, file.GetName(), 511);

			if (auxio_load_archive(lastarchname, &compbuffer, &compsize, &compoffset, (const char *)mbstr))
			{
				file.SetContent((const void*)&compbuffer[compoffset], (ulong)compsize);

				free(compbuffer);
			}				
			break;

		case User::File::LOAD_SAMPLE:
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU:
		case User::File::LOAD_SAMPLE_MOERO_PRO_YAKYUU_88:
		case User::File::LOAD_SAMPLE_MOERO_PRO_TENNIS:
		case User::File::LOAD_SAMPLE_TERAO_NO_DOSUKOI_OOZUMOU:
		case User::File::LOAD_SAMPLE_AEROBICS_STUDIO:
			wcstombs(mbstr, file.GetName(), 511);

			if (auxio_load_archive(lastarchname, &compbuffer, &compsize, &compoffset, (const char *)mbstr))
			{
				int chan, bits, rate;

				if (!strncmp((const char *)compbuffer, "RIFF", 4))
				{
					chan = compbuffer[20] | compbuffer[21]<<8;
					rate = compbuffer[24] | compbuffer[25]<<8 | compbuffer[26]<<16 | compbuffer[27]<<24;
					bits = compbuffer[34] | compbuffer[35]<<8; 

//					std::cout << "WAV has " << chan << " chan, " << bits << " bits per sample, rate = " << rate << "\n";

					file.SetSampleContent((const void*)&compbuffer[compoffset], (ulong)compsize, (chan == 2) ? true : false, bits, rate);
				}

				free(compbuffer);
			}				
			break;

		case User::File::LOAD_BATTERY: // load in battery data from a file
		case User::File::LOAD_EEPROM: // used by some Bandai games, can be treated the same as battery files
		case User::File::LOAD_TAPE: // for loading Famicom cassette tapes
		case User::File::LOAD_TURBOFILE: // for loading turbofile data
		{
			int size;
			FILE *f;

			f = fopen(savename, "rb");
			if (!f)
			{
				return;
			}
			fseek(f, 0, SEEK_END);
			size = ftell(f);
			fclose(f);

			std::ifstream batteryFile( savename, std::ifstream::in|std::ifstream::binary );

			if (batteryFile.is_open())
			{
				file.SetContent( batteryFile );
			}
			break;
		}

		case User::File::SAVE_BATTERY: // save battery data to a file
		case User::File::SAVE_EEPROM: // can be treated the same as battery files
		case User::File::SAVE_TAPE: // for saving Famicom cassette tapes
		case User::File::SAVE_TURBOFILE: // for saving turbofile data
		{
			std::ofstream batteryFile( savename, std::ifstream::out|std::ifstream::binary );
			const void* savedata;
			unsigned long savedatasize;

			file.GetContent( savedata, savedatasize );

			if (batteryFile.is_open())
				batteryFile.write( (const char*) savedata, savedatasize );

			break;
		}

		case User::File::LOAD_FDS: // for loading modified Famicom Disk System files
		{
			char fdsname[512];

			sprintf(fdsname, "%s.ups", rootname);
			
			std::ifstream batteryFile( fdsname, std::ifstream::in|std::ifstream::binary );

			// no ups, look for ips
			if (!batteryFile.is_open())
			{
				sprintf(fdsname, "%s.ips", rootname);

				std::ifstream batteryFile( fdsname, std::ifstream::in|std::ifstream::binary );

				if (!batteryFile.is_open())
				{
					return;
				}

				file.SetPatchContent(batteryFile);
				return;
			}

			file.SetPatchContent(batteryFile);
			break;
		}

		case User::File::SAVE_FDS: // for saving modified Famicom Disk System files
		{
			char fdsname[512];

			sprintf(fdsname, "%s.ups", rootname);

			std::ofstream fdsFile( fdsname, std::ifstream::out|std::ifstream::binary );

			if (fdsFile.is_open())
				file.GetPatchContent( User::File::PATCH_UPS, fdsFile );

			break;
		}
	}
}

static void cleanup_after_io(void)
{
	gtk_main_iteration_do(FALSE);
	gtk_main_iteration_do(FALSE);
	gtk_main_iteration_do(FALSE);
	if (sSettings->GetFullscreen())
	{
		SetupVideo();
	}
}

int main(int argc, char *argv[])
{
	static SDL_Event event;
	int i;
	void* userData = (void*) 0xDEADC0DE;

	// read the key/controller mapping
	ctl_defs = parse_input_file();
	if (!ctl_defs)
	{
		std::cout << "Couldn't read ~/.nestopia/nstcontrols file\n";
		return -1;
	}

	playing = 0;
	intbuffer = NULL;

	auxio_init();

	sSettings = new Settings;
	sCheatMgr = new CheatMgr;

	UIHelp_Init(argc, argv, sSettings, sCheatMgr);
	
	// setup video lock/unlock callbacks
	Video::Output::lockCallback.Set( VideoLock, userData );
	Video::Output::unlockCallback.Set( VideoUnlock, userData );

	// misc callbacks (for others, see NstApuUser.hpp)
	User::fileIoCallback.Set( DoFileIO, userData );
	User::logCallback.Set( DoLog, userData );

	// try to load the FDS BIOS
	auxio_set_fds_bios();

	// and the NST database
	auxio_load_db();

	// attempt to load and autostart a file specified on the commandline
	if (argc > 1)
	{
		NstLoadGame(argv[1]);

		if (loaded)
		{
			if (nsf_mode)
			{
				on_nsfplay_clicked(NULL, NULL);
			}
			else
			{
				on_playbutton_clicked(NULL, NULL);
			}
		}
	}

	nst_quit = 0;
	while (!nst_quit)
	{
		if (playing)
		{
			if (nsf_mode)
			{
				gtk_main_iteration_do(FALSE);
			}
			else
			{
			 	while (SDL_PollEvent(&event))
				{
					switch (event.type)
					{
						case SDL_QUIT:
							schedule_stop = 1;
							break;

						case SDL_KEYDOWN:
						case SDL_KEYUP:
							// ignore num lock, caps lock, and "mode" (whatever that is)
							event.key.keysym.mod = (SDLMod)((int)event.key.keysym.mod & (~(KMOD_NUM | KMOD_CAPS | KMOD_MODE)));
							
							// (intentional fallthrough)
						case SDL_JOYAXISMOTION:
					    case SDL_JOYBUTTONDOWN:
						case SDL_JOYBUTTONUP:
							nst_dispatch(cNstPads, event);
							break;
					}	
				}

				if (NES_SUCCEEDED(Rewinder(emulator).Enable(true)))
				{
					Rewinder(emulator).EnableSound(true);
				}
			}

			m1sdr_TimeCheck();
			if (updateok)
			{
				emulator.Execute(cNstVideo, cNstSound, cNstPads);
				updateok = 0;
			}

			if (state_save)
			{
				kill_video_if_fs();
				auxio_do_state_save();
				state_save = 0;
				cleanup_after_io();
			}

			if (state_load)
			{
				kill_video_if_fs();
				auxio_do_state_load();
				state_load = 0;
				cleanup_after_io();
			}

			if (movie_load)
			{
				kill_video_if_fs();
				auxio_do_movie_load();
				movie_load = 0;
				cleanup_after_io();
			}

			if (movie_save)
			{
				kill_video_if_fs();
				auxio_do_movie_save();
				movie_load = 0;
				cleanup_after_io();
			}

			if (movie_stop)
			{
				movie_stop = 0;
				auxio_do_movie_stop();
			}

			if (schedule_stop)
			{
				NstStopPlaying();
			}
		}
		else
		{
			gtk_main_iteration_do(FALSE);
		}
	}

	nst_unload2();

	auxio_shutdown();

	delete sSettings;
	delete sCheatMgr;

	write_output_file(ctl_defs);
	free(ctl_defs);

	return 0;
}

void SetupVideo()
{
	// renderstate structure
	Video::RenderState renderState;
	Machine machine( emulator );
	Cartridge::Database database( emulator );
	Video::RenderState::Filter filter;
	int scalefactor = sSettings->GetScaleAmt() + 1;
	int i;

	// init SDL
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
	{
		std::cout << "Unable to init SDL\n";
		return;
	}

	// figure out the region
	framerate = 60;
	if (sSettings->GetVideoMode() == 2)		// force PAL
	{
		machine.SetMode(Machine::PAL);
		framerate = 50;
	}
	else if (sSettings->GetVideoMode() == 1) 	// force NTSC
	{
		machine.SetMode(Machine::NTSC);
	}
	else	// auto
	{
		if (database.IsLoaded())
		{
			if (dbentry.GetSystem() == Cartridge::Profile::System::NES_PAL)
			{
				machine.SetMode(Machine::PAL);
				framerate = 50;
			}
			else
			{
				machine.SetMode(Machine::NTSC);
			}
		}
		else
		{
			machine.SetMode(machine.GetDesiredMode());
		}
	}

	// we don't create a window in NSF mode
	if (nsf_mode) 
	{
		return;
	}

	if (SDL_NumJoysticks() > 0)
	{
		for (i = 0; i < SDL_NumJoysticks(); i++)
		{
			// we only? support 10 joysticks
			if (i < 10)
			{
				joy[i] = SDL_JoystickOpen(i);
			}
		}

		SDL_JoystickEventState(SDL_ENABLE);	// turn on regular updates
	}

	// compute the major video parameters from the scaler type and scale factor
	switch (sSettings->GetScale())
	{
		case 0:	// None (no scaling unless OpenGL)
			if (sSettings->GetRenderType() == 0)
			{
				if (scalefactor > 1)
				{
					std::cout << "Warning: raw scale factors > 1 not allowed with pure software, use OpenGL\n";
				}
				cur_width = cur_Rwidth = Video::Output::WIDTH;
				cur_height = cur_Rheight = Video::Output::HEIGHT;
			}
			else
			{
				cur_width = Video::Output::WIDTH;
				cur_height = Video::Output::HEIGHT;
				cur_Rwidth = cur_width * scalefactor;
				cur_Rheight = cur_height * scalefactor;
			}
			filter = Video::RenderState::FILTER_NONE;
			break;

		case 1: // NTSC
			if (sSettings->GetRenderType() == 0)
			{
				if (scalefactor > 1)
				{
					std::cout << "Warning: NTSC scale factors > 1 not allowed with pure software - use OpenGL\n";
				}

				scalefactor = 1;
			}

			cur_width = Video::Output::NTSC_WIDTH;
			cur_Rwidth = cur_width * scalefactor;
			cur_height = Video::Output::HEIGHT;
			cur_Rheight = cur_height * 2 * scalefactor;
			filter = Video::RenderState::FILTER_NTSC;
			break;

		case 2: // scale x
			if (scalefactor == 4) 
			{
				std::cout << "Warning: Scale x only allows scale factors of 3 or less\n";
				scalefactor = 3;	// there is no scale4x
			}

			cur_width = cur_Rwidth = Video::Output::WIDTH * scalefactor;
			cur_height = cur_Rheight = Video::Output::HEIGHT * scalefactor;

			switch (scalefactor)
			{
				case 2:
					filter = Video::RenderState::FILTER_SCALE2X;
					break;

				case 3:
					filter = Video::RenderState::FILTER_SCALE3X;
					break;

				default:
					filter = Video::RenderState::FILTER_NONE;
					break;
			}
			break;

		case 3: // scale HQx
			cur_width = cur_Rwidth = Video::Output::WIDTH * scalefactor;
			cur_height = cur_Rheight = Video::Output::HEIGHT * scalefactor;

			switch (scalefactor)
			{
				case 2:
					filter = Video::RenderState::FILTER_HQ2X;
					break;

				case 3:
					filter = Video::RenderState::FILTER_HQ3X;
					break;

				case 4:
					filter = Video::RenderState::FILTER_HQ4X;
					break;

				default:
					filter = Video::RenderState::FILTER_NONE;
					break;
			}
			break;
	}



	int eFlags = SDL_HWSURFACE;
        using_opengl = (sSettings->GetRenderType() > 0);
	linear_filter = (sSettings->GetRenderType() == 2);
	if (using_opengl)
	{
		SDL_GL_SetAttribute( SDL_GL_DOUBLEBUFFER, 1 );
		eFlags = SDL_OPENGL;
	}

	if (sSettings->GetFullscreen())
	{
		screen = SDL_SetVideoMode(cur_Rwidth, cur_Rheight, 16, SDL_ANYFORMAT | SDL_DOUBLEBUF | SDL_FULLSCREEN | eFlags);
	}
	else
	{
		screen = SDL_SetVideoMode(cur_Rwidth, cur_Rheight, 16, SDL_ANYFORMAT | SDL_DOUBLEBUF | eFlags);
	}

	if (!screen)
	{
		std::cout << "SDL couldn't set video mode\n";
		exit(-1);
	}

	renderState.filter = filter;
	renderState.width = cur_width;
	renderState.height = cur_height;

	// example configuration
	if (using_opengl)
	{
		opengl_init_structures();

		renderState.bits.count = 32;
		renderState.bits.mask.r = 0x00ff0000;
		renderState.bits.mask.g = 0x0000ff00;
		renderState.bits.mask.b = 0x000000ff;
	}
	else
	{
		renderState.bits.count = screen->format->BitsPerPixel;
		renderState.bits.mask.r = screen->format->Rmask;
		renderState.bits.mask.g = screen->format->Gmask;
		renderState.bits.mask.b = screen->format->Bmask;
	}

	// allocate the intermediate render buffer
	intbuffer = malloc(renderState.bits.count * renderState.width * renderState.height);

	// acquire the video interface
	Video video( emulator );

	// set the sprite limit
	video.EnableUnlimSprites(sSettings->GetSprlimit() ? false : true);

	// set up the NTSC type
	switch (sSettings->GetNtscMode())
	{
		case 0:	// composite
			video.SetSharpness(Video::DEFAULT_SHARPNESS_COMP);
			video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_COMP);
			video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_COMP);
			video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_COMP);
			video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_COMP);
			break;

		case 1:	// S-Video
			video.SetSharpness(Video::DEFAULT_SHARPNESS_SVIDEO);
			video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_SVIDEO);
			video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_SVIDEO);
			video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_SVIDEO);
			video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_SVIDEO);
			break;

		case 2:	// RGB
			video.SetSharpness(Video::DEFAULT_SHARPNESS_RGB);
			video.SetColorResolution(Video::DEFAULT_COLOR_RESOLUTION_RGB);
			video.SetColorBleed(Video::DEFAULT_COLOR_BLEED_RGB);
			video.SetColorArtifacts(Video::DEFAULT_COLOR_ARTIFACTS_RGB);
			video.SetColorFringing(Video::DEFAULT_COLOR_FRINGING_RGB);
			break;
	}
			     
	// set the render state
	if (NES_FAILED(video.SetRenderState( renderState )))
	{
		std::cout << "NEStopia core rejected render state\n";
		::exit(0);
	}

	if (sSettings->GetFullscreen())
	{
		SDL_ShowCursor(0);
	}
}

// initialize sound going into the game
void SetupSound()
{
	// acquire interface
	Sound sound( emulator );

	lnxdrv_apimode = sSettings->GetSndAPI();

	m1sdr_Init(sSettings->GetRate());
	m1sdr_SetCallback((void *)nst_do_frame);
	m1sdr_PlayStart();

	// init DSP module
	seffect_init(sSettings);

	// example configuration (these are the default values)
	sound.SetSampleBits( 16 );
	sound.SetSampleRate(sSettings->GetRate());
	sound.SetVolume(Sound::ALL_CHANNELS, sSettings->GetVolume());
	if (sSettings->GetStereo())
	{
		sound.SetSpeaker( Sound::SPEAKER_STEREO );
	}
	else
	{
		sound.SetSpeaker( Sound::SPEAKER_MONO );
	}
}

// initialize input going into the game
void SetupInput()
{
	// connect a standard NES pad onto the first port
	Input(emulator).ConnectController( 0, Input::PAD1 );

	// connect a standard NES pad onto the second port too
	Input(emulator).ConnectController( 1, Input::PAD2 );
}

void configure_savename( const char* filename )
{
	int i = 0;

	strcpy(savename, filename);

	// strip the . and extention off the filename for saving
	for (i = strlen(savename)-1; i > 0; i--)
	{
		if (savename[i] == '.')
		{
			savename[i] = '\0';
			break;
		}
	}

	strcpy(capname, savename);
	strcpy(gamebasename, savename);

	// strip the path off the savename to get the filename only
	for (i = strlen(capname)-1; i > 0; i--)
	{
		if (capname[i] == '/')
		{
			strcpy(capname, &capname[i+1]);
			break;
		}
	}


	// also generate the window caption
	sprintf(caption, "NEStopia %s: %s", NST_VERSION, capname);

	strcpy(rootname, savename);
	strcat(savename, ".sav");
}

// try and find a patch for the game being loaded
static int find_patch(char *patchname)
{
	FILE *f;

	// did the user turn off auto softpatching?
	if (sSettings->GetSoftPatch() == 0)
	{
		return 0;
	}

	snprintf(patchname, 511, "%s.ips", gamebasename);
	if ((f = fopen(patchname, "rb")) != NULL)
	{
		fclose(f);
		return 1;
	}
	else
	{
		snprintf(patchname, 511, "%s.ups", gamebasename);
		if ((f = fopen(patchname, "rb")) != NULL)
		{
			fclose(f);
			return 1;
		}
	}

	return 0;
}

// load a game or NES music file
void NstLoadGame(const char* filename)
{
	// acquire interface to machine
	Cartridge::Database database( emulator );
	Machine machine( emulator );
	Nsf nsf( emulator );
	Nes::Result result;
	unsigned char *compbuffer;
	int compsize, wascomp, compoffset;
	char gamename[512], patchname[512];

	if (nsf_mode)
	{
		Nsf nsf( emulator );
	
		nsf.StopSong();

		// clear the audio buffer
		memset(lbuf, 0, sizeof(lbuf));

		playing = 0;
	}

	// unload if necessary
	nst_unload();

	// (re)configure savename
	configure_savename(filename);

	// check if it's an archive
	wascomp = 0;
	if (auxio_load_archive(filename, &compbuffer, &compsize, &compoffset, NULL, gamename))
	{
		std::istrstream file((char *)compbuffer+compoffset, compsize);
		wascomp = 1;

		strncpy(lastarchname, filename, 511);
	
		configure_savename(gamename);

		if (database.IsLoaded())
		{
			dbentry = database.FindEntry((void *)&compbuffer[compoffset], compsize, get_favored_system());
		}

		if (find_patch(patchname))
		{
			std::ifstream pfile(patchname, std::ios::in|std::ios::binary);

			Machine::Patch patch(pfile, false);

			// load game and softpatch
			result = machine.Load( file, get_favored_system(), patch );
		}
		else
		{
			// load game
			result = machine.Load( file, get_favored_system() );
		}
	}
	else
	{
		FILE *f;
		int length;
		unsigned char *buffer;

		// this is a little ugly
		if (database.IsLoaded())
		{
			f = fopen(filename, "rb");
			if (!f)
			{
				loaded = 0;
				UIHelp_Unload();
				return;
			}

			fseek(f, 0, SEEK_END);
			length = ftell(f);
			fseek(f, 0, SEEK_SET);

			buffer = (unsigned char *)malloc(length);
			fread(buffer, length, 1, f);
			fclose(f);

			dbentry = database.FindEntry(buffer, length, get_favored_system());

			free(buffer);
		}
	
		configure_savename(filename);

		// C++ file stream
		std::ifstream file(filename, std::ios::in|std::ios::binary);

		if (find_patch(patchname))
		{
			std::ifstream pfile(patchname, std::ios::in|std::ios::binary);

			Machine::Patch patch(pfile, false);

			// load game and softpatch
			result = machine.Load( file, get_favored_system(), patch );
		}
		else
		{
			// load game
			result = machine.Load( file, get_favored_system() );
		}
	}

	// failed?
	if (NES_FAILED(result))
	{
		switch (result)
		{
			case Nes::RESULT_ERR_INVALID_FILE:
				std::cout << "Invalid file\n";
				break;

			case Nes::RESULT_ERR_OUT_OF_MEMORY:
				std::cout << "Out of memory\n";
				break;

			case Nes::RESULT_ERR_CORRUPT_FILE:
				std::cout << "Corrupt or missing file\n";
				break;

			case Nes::RESULT_ERR_UNSUPPORTED_MAPPER:
				std::cout << "Unsupported mapper\n";
				break;

			case Nes::RESULT_ERR_MISSING_BIOS:
				std::cout << "Can't find disksys.rom for FDS game\n";
				break;

			default:
				std::cout << "Unknown error #" << result << "\n";
				break;
		}

		return;
	}

	// free the buffer if necessary
	if (wascomp)
	{
		free(compbuffer);
	}

	// is this an NSF?
	nsf_mode = (machine.Is(Machine::SOUND)) ? 1 : 0;

	if (nsf_mode)
	{
		// update the UI
		UIHelp_NSFLoaded();

		// initialization
		SetupVideo();
		SetupSound();
		SetupInput();

		cNstVideo = new Video::Output;
		cNstSound = new Sound::Output;
		cNstPads  = new Input::Controllers;

		cNstSound->samples[0] = lbuf;
		cNstSound->length[0] = sSettings->GetRate()/framerate;
		cNstSound->samples[1] = NULL;
		cNstSound->length[1] = 0;

		m1sdr_SetSamplesPerTick(cNstSound->length[0]);

		updateok = 0;
		playing = 1;
		schedule_stop = 0;
	}
	else
	{
		UIHelp_GameLoaded();

		if (machine.Is(Machine::DISK))
		{
			Fds fds( emulator );

			fds.InsertDisk(0, 0);
		}
	}

	// note that something is loaded
	loaded = 1;

	// power on
	machine.Power( true ); // false = power off
}


