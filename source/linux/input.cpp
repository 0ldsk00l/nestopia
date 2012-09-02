/*
	NEStopia / Linux
	Port by R. Belmont
	
	input.cpp - input handling
*/

#include <iostream>
#include <fstream>
#include <SDL.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "core/api/NstApiEmulator.hpp"
#include "core/api/NstApiVideo.hpp"
#include "core/api/NstApiSound.hpp"
#include "core/api/NstApiMachine.hpp"
#include "core/api/NstApiUser.hpp"
#include "core/api/NstApiNsf.hpp"
#include "core/api/NstApiMovie.hpp"
#include "core/api/NstApiFds.hpp"
#include "core/api/NstApiRewinder.hpp"
#include "oss.h"
#include "settings.h"
#include "auxio.h"
#include "input.h"
#include "kentry.h"

static char linebuf[256];
static FILE *infile;


// translate player control to Nes::Api code
static int translate_code(const char *str)
{
	int i = kentry_find_str(controlcodes, str);
	if (i == -1)
		std::cout << "Error understanding control type '" << str << "'\n";
	return i;
}

// translate key descriptor into SDL keycode
static bool translate_key(const char *substr, SDL_Event &evt)
{
	evt.type = SDL_KEYDOWN;

	evt.key.keysym.sym = (SDLKey)0;
	evt.key.keysym.mod = (SDLMod)0;

	if (substr[0] != '_')	// literal case
	{
		evt.key.keysym.sym = SDLKey(tolower(substr[0]));
		return true;
	}

	if (substr[1] == '_')	// quoted _
	{
		evt.key.keysym.sym = SDLKey('_');
		return true;
	}

	int i = kentry_find_str(keycodes, substr + 1);

	if (i == -1)
	{
		std::cout << "unknown key " << substr << "\n";
		return false;
	}

	evt.key.keysym.sym = SDLKey(i);
	return true;
}


// translate event type into InputEvtT value
static int translate_meta(const char *str)
{
	int i = kentry_find_str(metacodes, str);
	if (i == -1)
	{
		std::cout << "invalid event type '" << str << "'\n";
	}

	return i;
}


// add a control to the list, fail if we're full
bool add_control(const InputDefT &control, InputDefT *pcontrol, int &icontrol, int ccontrol)
{
	if (icontrol >= ccontrol)
	{
		std::cout << "Too many controls (max " << ccontrol << ")\n";
		return false;
	}
	memcpy(pcontrol + icontrol++, &control, sizeof(control));

	return true;
}


// translate joystick/gamepad spec:
// _<num> ( A<axis #><PLUS|MINUS|+|-> | B<button #> )
static bool translate_joy(const char* str, SDL_Event &evt)
{
	if (!isdigit(str[0]))
	{
		std::cout << "Bad joypad number '" << str[0] << "' (expected 0-9)\n";
		return false;
	}
	evt.jbutton.which = str[0] - '0';

	if (str[1] == 'A')		// axis
	{
		evt.type = SDL_JOYAXISMOTION;
		evt.jaxis.axis = str[2] - '0';
		evt.jaxis.value = (str[3] == 'M' || str[3] == '-') ? -1 : 1;
	}
	else if (str[1] == 'B')	// button
	{
		evt.type = SDL_JOYBUTTONDOWN;
		evt.jbutton.button = str[2] - '0';
		if (isdigit(str[3]))	// 2 digit button number?
			evt.jbutton.button = evt.jbutton.button * 10 + str[3] - '0';
	}
	else
	{
		std::cout << "Malformed joypad widget (A for axis, B for button)\n";
		return false;
	}

	return true;
}


// helper for translate_event: punctuation or alphanumeric is valid
inline static bool isvalid(char c)
{
	return isalnum(c) || ispunct(c);
}


// translate an input line event into an InputDefT
// returns true on success, false on failure (and outputs error message)
bool translate_event(char *linebuf, InputDefT *pcontrol, int &icontrol, int ccontrol)
{
	InputDefT control;

	// ensure that unused fields come out as 0
	memset(&control, 0, sizeof(control));

	// break line into "words" and insert null terminators
	char *pbuf = linebuf;
	const char *words[10] = { 0 };

	for (int iword = 0; iword < CRg(words) - 1 && isvalid(*pbuf); ++iword)
	{
		words[iword] = pbuf;
		for ( ; isvalid(*pbuf); ++pbuf);
		if (*pbuf == '\0')
		{
			break;
		}
		*pbuf++ = '\0';
		for ( ; isblank(*pbuf); ++pbuf);
	}

	int iword = 0;
	const char *word = words[iword++];

	// format is: <event> <key...> (where "key" could be a joystick button/axis)
	//
	// 'P<digits><code>' (e.g. P1START) means a player event
	if (word[0] == 'P' && isdigit(word[1]))
	{
		control.player = word[1] - '0';
		if (control.player < 1 || control.player > 4)
		{
			std::cout << "Invalid player number (" << word[1] << ") in file\n";
			return false;
		}
		control.codeout = translate_code(word + 2);
	}
	// non-player ("meta") event
	else
	{
		control.player = 0; 	// non-player code
		control.codeout = translate_meta(word);
	}

	if (control.codeout == -1)
	{
		return false;
	}

	// now reading key(s) or joystick events
	while ((word = words[iword++]) != NULL)
	{
		if (word[0] == '_' && word[1] == 'J')
		{
			if (!translate_joy(word + 2, control.evt))
			{
				std::cout << "Malformed joypad spec " << word << "\n";
				return false;
			}
		}
		else
		{
			if (!translate_key(word, control.evt))
			{
				std::cout << "Malformed key spec " << word << "\n";
				return false;
			}
		}

		if (!add_control(control, pcontrol, icontrol, ccontrol))
		{
			return false;
		}

		// clear for the next one
		memset(&control.evt, 0, sizeof(control.evt));
	}
	return true;
}


const int ccontrolMax = 128;

// parse the "nstcontrols" file
InputDefT *parse_input_file()
{
	InputDefT *controls;
	int curentry = 0, i, j;
	char substr[256];
	char dirname[1024], *home;

	home = getenv("HOME");
	sprintf(dirname, "%s/.nestopia/nstcontrols", home);

 	infile = fopen(dirname, "r");
	if (!infile)
	{
		return NULL;
	}

	controls = (InputDefT *)malloc(sizeof(InputDefT) * ccontrolMax);
	memset(controls, 0, sizeof(InputDefT) * ccontrolMax);

	// pre-insert ALT + ENTER fullscreen toggle since we don't 
	// support requiring multiple keys for one action
	controls[curentry].evt.type = SDL_KEYDOWN;
	controls[curentry].evt.key.keysym.sym = SDLK_RETURN;
	controls[curentry].evt.key.keysym.mod = KMOD_LALT;
	controls[curentry].player = 0;
	controls[curentry++].codeout = FSCREEN;

	memset(&controls[curentry], 0, sizeof(controls[0]));

	while (!feof(infile))
	{
		fgets(linebuf, 256, infile);

		if ((linebuf[0] != ';') && isalnum(linebuf[0]))
		{
			// leave space for the terminating entry
			if (!translate_event(linebuf, controls, curentry, ccontrolMax - 1))
			{
				goto LFail;
			}
		}
	}

	controls[curentry].player = -1;
	return controls;

LFail:
	free(controls);
	return NULL;
}

// write out the "nstcontrols" file
void write_output_file(InputDefT *ctl_defs)
{
	FILE *outfile;
	char dirname[1024], *home;
	int i, evt;
	const char *mc;

	home = getenv("HOME");
	sprintf(dirname, "%s/.nestopia/nstcontrols", home);

 	outfile = fopen(dirname, "w");
	if (!outfile)
	{
		std::cout << "Could not write ~/.nestopia/nstcontrols file.  Any changes will be lost.\n";
		return;
	}

	// write a header
	fprintf(outfile, ";\n");
	fprintf(outfile, "; NEStopia/Linux controls file\n");
	fprintf(outfile, "; Generated by NEStopia/Linux.  Be careful editing!\n");
	fprintf(outfile, ";\n\n");

	i = 1;	// skip the artifical fullscreen toggle entry
	while (ctl_defs[i].player != -1) 
	{
		char type[16], ctrl[32];

		// is this a player control or a metacontrol?
		if (ctl_defs[i].player == 0) 
		{
			// metacontrol
			mc = kentry_find_code(metacodes, ctl_defs[i].codeout);

			if (mc) 
			{
				sprintf(type, "%s\t", mc);
			}
			else
			{
				std::cout << "Unable to find string for metacode!\n";
			}
		}
		else
		{
			mc = kentry_find_code(controlcodes, ctl_defs[i].codeout);

			sprintf(type, "P%d%s\t", ctl_defs[i].player, mc);
		}
	
		//  is this a keyboard control?
		if (ctl_defs[i].evt.type == SDL_KEYDOWN)
		{
			mc = kentry_find_code(keycodes, ctl_defs[i].evt.key.keysym.sym);
			if (mc != NULL) 
			{
				sprintf(ctrl, "_%s", mc);
			}
			else
			{
				sprintf(ctrl, "%c", ctl_defs[i].evt.key.keysym.sym);
			}
		}
  		else if (ctl_defs[i].evt.type == SDL_JOYAXISMOTION) 
		{
			sprintf(ctrl, "_J%dA%d%s\t", ctl_defs[i].evt.jbutton.which, ctl_defs[i].evt.jaxis.axis, (ctl_defs[i].evt.jaxis.value == 1) ? "PLUS" : "MINUS");
		}
		else if (ctl_defs[i].evt.type == SDL_JOYBUTTONDOWN)
		{
			sprintf(ctrl, "_J%dB%d\t", ctl_defs[i].evt.jbutton.which, ctl_defs[i].evt.jbutton.button);
		}
		else
		{
			std::cout << "Undefined event type!\n";
		}

		fprintf(outfile, "%s\t%s\n", type, ctrl);
		i++;
	}

	fclose(outfile);
}
