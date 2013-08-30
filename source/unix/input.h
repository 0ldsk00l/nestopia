#ifndef _INPUT_H_
#define _INPUT_H_

#define NUMGAMEPADS 2
#define NUMBUTTONS 8
#define TOTALBUTTONS (NUMGAMEPADS*NUMBUTTONS)
#define DEADZONE (32768/3)

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
	
	SDL_Event ju;
	SDL_Event jd;
	SDL_Event jl;
	SDL_Event jr;
	SDL_Event jselect;
	SDL_Event jstart;
	SDL_Event ja;
	SDL_Event jb;
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
	
	gchar *js_p1u;
	gchar *js_p1d;
	gchar *js_p1l;
	gchar *js_p1r;
	gchar *js_p1select;
	gchar *js_p1start;
	gchar *js_p1a;
	gchar *js_p1b;
	
	// Player 2
	gchar *kb_p2u;
	gchar *kb_p2d;
	gchar *kb_p2l;
	gchar *kb_p2r;
	gchar *kb_p2select;
	gchar *kb_p2start;
	gchar *kb_p2a;
	gchar *kb_p2b;
	
	gchar *js_p2u;
	gchar *js_p2d;
	gchar *js_p2l;
	gchar *js_p2r;
	gchar *js_p2select;
	gchar *js_p2start;
	gchar *js_p2a;
	gchar *js_p2b;
} inputsettings;

typedef struct {
	unsigned char player;
	unsigned char nescode;
	unsigned char pressed;
} nesinput;

void input_init();
void input_deinit();
void input_process(Input::Controllers *controllers, SDL_Event event);
void input_inject(Input::Controllers *controllers, nesinput input);
void input_match_keyboard(Input::Controllers *controllers, SDL_Event event);
void input_match_joystick(Input::Controllers *controllers, SDL_Event event);
char* input_translate_event(SDL_Event event);
SDL_Event input_translate_string(char *string);
int input_checksign(int axisvalue);
void input_read_config();
void input_write_config();
void input_set_default();

#endif
