#ifndef _INPUT_H_
#define _INPUT_H_

#define NUMGAMEPADS 2

#include <glib.h>
#include <SDL.h>
#include "core/api/NstApiInput.hpp"

using namespace Nes::Api;

typedef struct {
	SDL_Scancode u;
	SDL_Scancode d;
	SDL_Scancode l;
	SDL_Scancode r;
	SDL_Scancode select;
	SDL_Scancode start;
	SDL_Scancode a;
	SDL_Scancode b;
} gamepad;

typedef struct {
	
	// Player 1
	gchar *kb_p1u;
	gchar *kb_p1d;
	gchar *kb_p1l;
	gchar *kb_p1r;
	gchar *kb_p1select;
	gchar *kb_p1start;
	gchar *kb_p1a;
	gchar *kb_p1b;
	
	// Player 2
	gchar *kb_p2u;
	gchar *kb_p2d;
	gchar *kb_p2l;
	gchar *kb_p2r;
	gchar *kb_p2select;
	gchar *kb_p2start;
	gchar *kb_p2a;
	gchar *kb_p2b;
} inputsettings;

void input_init();
void input_process(Input::Controllers *controllers, SDL_Event event);
void input_read_config();

#endif
