/*
	NEStopia / Linux
	Port by R. Belmont
	
	controlconfig.cpp - controller configuration utility.

	Based on a patch by David B. Robins.
	Font code and data by Duddie.
*/

#include <iostream>
#include <fstream>
#include <strstream>
#include <sstream>
#include <iomanip>
#include <string>
#include <vector>
#include <SDL.h>
#include <string.h>
#include <cassert>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include "kentry.h"
#include "input.h"
#include "controlconfig.h"

#include "fonts.inc"

static SDL_Surface *config_screen;

static const int win_w = 360;
static const int win_h = 64;

struct ConfigItem
{
	const KEntry *table;
	int	taboffs;
	int	player;
};

// Order MUST match the items in the config popup
static ConfigItem config[] =
{
	{ controlcodes, 0, 1 },	// p1 up
	{ controlcodes, 1, 1 }, // p1 down
	{ controlcodes, 2, 1 }, // p1 left
	{ controlcodes, 3, 1 }, // p1 right
	{ controlcodes, 4, 1 }, // p1 a
	{ controlcodes, 5, 1 }, // p1 b
	{ controlcodes, 6, 1 }, // p1 start
	{ controlcodes, 7, 1 }, // p1 select
	{ controlcodes, 0, 2 }, // p2 up    
	{ controlcodes, 1, 2 }, // p2 down  
	{ controlcodes, 2, 2 }, // p2 left  
	{ controlcodes, 3, 2 }, // p2 right 
	{ controlcodes, 4, 2 }, // p2 a     
	{ controlcodes, 5, 2 }, // p2 b     
	{ controlcodes, 6, 2 }, // p2 start 
	{ controlcodes, 7, 2 }, // p2 select
	{ metacodes, 7,  0 }, // movie save
	{ metacodes, 8,  0 }, // movie load
	{ metacodes, 9,  0 }, // movie stop
	{ metacodes, 0,  0 }, // reset
	{ metacodes, 10, 0 }, // FDS flip
	{ metacodes, 5, 0 },  // save state
	{ metacodes, 6, 0 },  // load state
	{ metacodes, 11, 0 }, // fullscreen toggle
	{ metacodes, 14, 0 }, // stop
	{ metacodes, 15, 0 }, // exit
	{ metacodes, 12, 0 }, // rewinder start
	{ metacodes, 13, 0 }, // rewinder stop
	{ metacodes, 1, 0 },  // quicksave slot 1
	{ metacodes, 3, 0 },  // quicksave slot 2
	{ metacodes, 2, 0 },  // quickload slot 1
	{ metacodes, 4, 0 },  // quickload slot 2
	{ metacodes, 16, 0 }, // Vs. coin 1
	{ metacodes, 17, 0 }, // Vs. coin 2
};

// represents an axis that is not centered in the "dead zone"
struct AxisNoise
{
	int which;
	int axis;
	Sint16 pos;

	AxisNoise(int _which, int _axis, Sint16 _pos) :
		which(_which), axis(_axis), pos(_pos) {}
	
	bool match(const SDL_Event &evt) const
	{
		assert(evt.type == SDL_JOYAXISMOTION);
		return evt.jaxis.which == which && evt.jaxis.axis == axis
			&& abs(evt.jaxis.value - pos) <= DEADZONE;
	}
};

typedef std::vector<AxisNoise> AxisNoiseVec;
static AxisNoiseVec axisNoise;

struct EventMatch
{
	SDL_Event evt;
	int len;			// of the user text
	std::string code;		// code for the nstcontrols file

	EventMatch(const SDL_Event &_evt, int _len, const char *_code) :
		evt(_evt), len(_len), code(_code) {}
};
typedef std::vector<EventMatch> EventVec;


// Put a glyph at the specified coordinates (surface must be locked!)
static inline void _putglyph(char *p, int Bpp, int pitch, char which)
{
	int *glyph = system_font[which & 0x7f];
	int x = 0, i;

	for(; *glyph != -1; ++glyph)
	{
		p += (((x += *glyph) >> 3) * pitch); x &= 7;
		for(i = 0; i < Bpp; ++i) p[(x * Bpp) + i] = 0xff;
	}
}

void sdl_locksurf()
{
	if (SDL_MUSTLOCK(config_screen)) 
	{
		SDL_LockSurface(config_screen);
	}
}

void sdl_unlockandflip()
{
	if(SDL_MUSTLOCK(config_screen))
	{
		SDL_UnlockSurface(config_screen);
	}

	SDL_Flip(config_screen);
}

void sdl_clear()
{
	memset((char *)config_screen->pixels, 0, config_screen->pitch * win_h);
}

// This writes a string of text at the given x and y coordinates
void sdl_font_text(int x, int y, const char *message)
{
	int pitch = config_screen->pitch, Bpp = config_screen->format->BytesPerPixel;
	char *p = (char*)config_screen->pixels + (pitch * y) + (Bpp * x);

	for(; *message; p += (8 * Bpp), ++message)
		_putglyph(p, Bpp, pitch, *message);
}

// check for noisy axis that shouldn't count as motion
inline static bool noisy_axis(const SDL_Event &evt)
{
	for (AxisNoiseVec::const_iterator i = axisNoise.begin();
		i != axisNoise.end(); ++i)
	{
		if (i->match(evt))
			return true;
	}
	return false;
}


// have we already used this input (or axis) for this event?
// (it's OK to assign the same key to multiple events, just not the same one!)
static bool match_event(const EventVec &evtvec, const SDL_Event &evt)
{
	for (EventVec::const_iterator i = evtvec.begin(); i != evtvec.end(); ++i)
	{
		const SDL_Event &evtT = i->evt;
		if (evtT.type != evt.type)
			continue;
		switch (evtT.type)
		{
		case SDL_KEYDOWN:
			if (evtT.key.keysym.sym == evt.key.keysym.sym)
				return true;
			break;

		case SDL_JOYBUTTONDOWN:
			if (evtT.jbutton.which == evt.jbutton.which
				&& evtT.jbutton.button == evt.jbutton.button)
				{
				return true;
				}
			break;
		
		case SDL_JOYAXISMOTION:
			if (evtT.jaxis.which == evt.jaxis.which
				&& evtT.jaxis.axis == evt.jaxis.axis)
				{
				return true;
				}
			break;
		}
	}
	return false;
}


// read an event from SDL and process it accordingly
static void read_event(InputDefT *ctl_list, const char *entry, const char *desc = NULL)
{
	char descstr[128], secs[32], curmatch[64];
	char lastdir = 0;
	int seconds = 5;
	int quit = 0;
	EventVec evtvec;	// input events already seen
	unsigned int basems, nextms;

	if (!desc)
	{
		desc = entry;
	}

	sprintf(secs, "Time left: %d", seconds);

	evtvec.clear();

	sdl_locksurf();
	sdl_clear();
	sprintf(descstr, "Press key or joy button/axis for: %s", desc);
	sdl_font_text(0, 0, descstr);
	sdl_font_text(0, 48, secs);
	sdl_unlockandflip();

	// use to ensure we don't record multiple events for same axis movement
	static struct
	{
		time_t t;		// time when movement counts as new
		int which;	// which joystick?
		int axis;		// which axis?
		char dir;		// direction ('+' or '-')
	}
	lastAxis;

	basems = SDL_GetTicks();
	nextms = basems + 1000;
	curmatch[0] = '\0';

	while (!quit)
	{
		SDL_Event evt;
		char match[20] = { 0 };
		char input[20] = { 0 };

		while (SDL_PollEvent(&evt))
		{
			// if we've already seen this input, ignore it
			if (match_event(evtvec, evt))
			{
				continue;
			}

			switch (evt.type)
			{
				case SDL_KEYDOWN:
				{
					int ksym = int(evt.key.keysym.sym);
					const char *key = kentry_find_code(keycodes, ksym);

					if (key != NULL)
					{
						sprintf(input, "_%s", key);
					}
					else if (isprint(ksym))
					{
						sprintf(input, "%c", ksym);
					}
					else
					{
						fprintf(stderr, "\a");	// beep
						break;
					}
					sprintf(match, "%s", SDL_GetKeyName(SDLKey(ksym)));
				}
				break;

				case SDL_JOYAXISMOTION:
					if (abs(evt.jaxis.value) > DEADZONE)
					{
						// check for a noisy axis with bad centering
						if (noisy_axis(evt))
							break;

						char dir = (evt.jaxis.value) > 0 ? '+' : '-';

						time_t t = time(NULL);

/*						if (lastAxis.dir == dir && lastAxis.which == evt.jaxis.which
							&& lastAxis.axis == evt.jaxis.axis && t < lastAxis.t)
							{
							break;
							}*/

						lastAxis.dir = dir;
						lastAxis.which = evt.jaxis.which;
						lastAxis.axis = evt.jaxis.axis;
						lastAxis.t = t + 2;

						sprintf(match, "joy %d axis %d %c",
							evt.jaxis.which + 1, evt.jaxis.axis, dir);
						sprintf(input, "_J%dA%d%c", evt.jaxis.which, evt.jaxis.axis, dir);
					}
					break;

				case SDL_JOYBUTTONDOWN:
					sprintf(match, "joy %d button %d",
						evt.jbutton.which + 1, evt.jbutton.button);
					sprintf(input, "_J%dB%d", evt.jbutton.which, evt.jbutton.button);
					break;

				case SDL_QUIT:
				 	return;
					break;
			}

			if (strlen(match) > 0) 
			{
				evtvec.clear();
				evtvec.push_back(EventMatch(evt, strlen(match), input));

				match[0] = '\0';
				for (EventVec::const_iterator i = evtvec.begin(); i != evtvec.end(); ++i)
				{
					strcat(match, i->code.c_str());
					strcat(match, " ");
				}

				if (match[0])
				{
					sdl_locksurf();
					sdl_clear();
					sdl_font_text(0, 0, descstr);
					sdl_font_text(0, 20, match);
					sdl_font_text(0, 48, secs);
					sdl_unlockandflip();

					strcpy(curmatch, match);
				}
			}
		}

		basems = SDL_GetTicks();
		if (basems >= nextms)
		{
			seconds--;
			if (seconds)
			{
				nextms = basems + 1000;

				sprintf(secs, "Time left: %d", seconds);

				sdl_locksurf();
				sdl_clear();
				sdl_font_text(0, 0, descstr);

				if (curmatch[0])
				{
					sdl_font_text(0, 20, curmatch);
				}
				sdl_font_text(0, 48, secs);
				sdl_unlockandflip();
			}
			else
			{
				quit = 1;
			}
		}
	}


	if (evtvec.size() > 0) 
	{
		char outline[128], tmp[32];
		InputDefT newcontrol;
		int icontrol = 0;

		// write the nstcontrols line
		sprintf(outline, "%s\t", desc);
		
		sprintf(tmp, "%s\t", evtvec[0].code.c_str());
		strcat(outline, tmp);

		// sanitize new control entry going in
		memset(&newcontrol, 0, sizeof(newcontrol));
	
		if (translate_event(outline, &newcontrol, icontrol, 1))
		{
			int j = 0;

			while (ctl_list[j].player != -1)
			{
				// if this is the same event type, player number, and bound control, replace it
				if ((ctl_list[j].evt.type == newcontrol.evt.type) && (ctl_list[j].player == newcontrol.player) && (ctl_list[j].codeout == newcontrol.codeout))
				{
					memcpy(&ctl_list[j], &newcontrol, sizeof(InputDefT));
					return;
				}
				
				j++;
			}

			// no existing entry was a match, append it to the list if there's room
			if (j < 128) 
			{
				memcpy(&ctl_list[j], &newcontrol, sizeof(InputDefT));
				ctl_list[j+1].player = -1;
			}
			else
			{
				std::cout << "Controls list too large, cannot add new control!\n";
			}
		}
	}
}


void run_configurator(InputDefT *ctl_list, int itemToSet, int usejoys)
{
	SDL_Joystick *joy[10];

	// set up SDL
	if (SDL_Init(SDL_INIT_VIDEO|SDL_INIT_JOYSTICK))
	{
		std::cout << "Unable to init SDL\n";
		return;
	}

	// set up joysticks
	if (usejoys)
	{
		int cjoy = SDL_NumJoysticks();
		fprintf(stderr, "%d joystick(s) detected.\n", cjoy);
		for (int ijoy = 0; ijoy < cjoy; ++ijoy)
		{
			if (ijoy < 10)
			{
				joy[ijoy] = SDL_JoystickOpen(ijoy);
				int caxis = SDL_JoystickNumAxes(joy[ijoy]);
				fprintf(stderr, "Joystick %d is '%s' has %d axes and %d buttons.\n",
				 ijoy + 1, SDL_JoystickName(ijoy), caxis, SDL_JoystickNumButtons(joy[ijoy]));

				// calibrate - read axis positions and store as "centered"
				// we expect joysticks to be centered in (-DEADZONE, DEADZONE) but
				// need to filter out noise
				for (int iaxis = 0; iaxis < caxis; ++iaxis)
				{
					Sint16 pos = SDL_JoystickGetAxis(joy[ijoy], iaxis);
					if (abs(pos) > DEADZONE)
						axisNoise.push_back(AxisNoise(ijoy, iaxis, pos));
				}
			}
		}
		SDL_JoystickEventState(SDL_ENABLE);
	}
	fprintf(stderr, "\n");

	// we need to set a video mode to get keyboard events
	if (! (config_screen = SDL_SetVideoMode(win_w, win_h, 16, SDL_DOUBLEBUF)))
	{
		std::cout << "SDL_SetVideoMode failed: %s\n";
		exit(1);
	}

	SDL_WM_SetCaption("Configure control", "nst");

	if (config[itemToSet].table == controlcodes)
	{
		std::string prefix = std::string("P") + std::string(1, '0' + config[itemToSet].player); 
		const KEntry *ctrl = &config[itemToSet].table[config[itemToSet].taboffs];

		std::string entry = prefix + ctrl->string;
		read_event(ctl_list, entry.c_str());
	}
	else
	{
		const KEntry *meta = &config[itemToSet].table[config[itemToSet].taboffs];

		read_event(ctl_list, "META", meta->string);
	}

	fprintf(stderr, "\n");
	printf("\n");

	if (usejoys)
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
		}
	}

	// cleanup SDL
	SDL_Quit();
}
