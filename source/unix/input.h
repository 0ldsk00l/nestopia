#ifndef _INPUT_H_
#define _INPUT_H_

#define NUMGAMEPADS 2
#define NUMBUTTONS 10
#define TOTALBUTTONS (NUMGAMEPADS*NUMBUTTONS)
#define DEADZONE (32768/3)

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
	SDL_Scancode ta;
	SDL_Scancode tb;
	
	SDL_Event ju;
	SDL_Event jd;
	SDL_Event jl;
	SDL_Event jr;
	SDL_Event jselect;
	SDL_Event jstart;
	SDL_Event ja;
	SDL_Event jb;
	SDL_Event jta;
	SDL_Event jtb;
} gamepad;

typedef struct {
	// Player 1
	char *kb_p1u;
	char *kb_p1d;
	char *kb_p1l;
	char *kb_p1r;
	char *kb_p1select;
	char *kb_p1start;
	char *kb_p1a;
	char *kb_p1b;
	char *kb_p1ta;
	char *kb_p1tb;
	
	char *js_p1u;
	char *js_p1d;
	char *js_p1l;
	char *js_p1r;
	char *js_p1select;
	char *js_p1start;
	char *js_p1a;
	char *js_p1b;
	char *js_p1ta;
	char *js_p1tb;
	
	// Player 2
	char *kb_p2u;
	char *kb_p2d;
	char *kb_p2l;
	char *kb_p2r;
	char *kb_p2select;
	char *kb_p2start;
	char *kb_p2a;
	char *kb_p2b;
	char *kb_p2ta;
	char *kb_p2tb;
	
	char *js_p2u;
	char *js_p2d;
	char *js_p2l;
	char *js_p2r;
	char *js_p2select;
	char *js_p2start;
	char *js_p2a;
	char *js_p2b;
	char *js_p2ta;
	char *js_p2tb;
} inputsettings;

typedef struct {
	unsigned char player;
	unsigned char nescode;
	unsigned char pressed;
	unsigned char turboa;
	unsigned char turbob;
} nesinput;

typedef struct {
	bool p1a;
	bool p1b;
	bool p2a;
	bool p2b;
} turbo;

void input_init();
void input_deinit();
void input_process(Input::Controllers *controllers, SDL_Event event);
void input_pulse_turbo(Input::Controllers *controllers);
void input_inject(Input::Controllers *controllers, nesinput input);
void input_match_keyboard(Input::Controllers *controllers, SDL_Event event);
void input_match_joystick(Input::Controllers *controllers, SDL_Event event);
char* input_translate_event(SDL_Event event);
SDL_Event input_translate_string(char *string);
int input_checksign(int axisvalue);
void input_config_read_new();
void input_config_read();
void input_config_write();
void input_set_default();
static int input_config_match(void* user, const char* section, const char* name, const char* value);

#endif
